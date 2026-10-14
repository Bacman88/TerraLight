#include "ota.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h>
#include <Update.h>
#include <esp_ota_ops.h>
#include <mbedtls/sha256.h>

#include "../core/version.h"

// Defer the core's automatic "mark valid" — we do it after OTA_HEALTHY_MS.
extern "C" bool verifyRollbackLater() { return true; }

namespace tl {
namespace ota {

namespace {

constexpr const char* BASE_URL = "https://github.com/" TL_GITHUB_REPO "/releases/";
constexpr uint32_t OTA_HEALTHY_MS = 60000;             // new image must survive this long
constexpr uint32_t FIRST_CHECK_DELAY_MS = 30000;       // after Wi-Fi comes up
constexpr uint32_t CHECK_INTERVAL_MS = 24UL * 3600000UL;
constexpr uint32_t RESTART_DELAY_MS = 3000;            // let the panel show "done"
constexpr uint32_t STALL_TIMEOUT_MS = 20000;
constexpr uint32_t TASK_STACK = 12288;                 // TLS handshake needs ~6 kB

enum class Job : uint8_t { Check, Install };

SemaphoreHandle_t g_mutex = nullptr;
State g_state = State::Idle;
String g_latest, g_tag, g_sha, g_error;
uint32_t g_size = 0;
volatile uint8_t g_progress = 0;
volatile bool g_busy = false;
bool g_pendingVerify = false;
uint32_t g_nextCheckMs = 0;  // 0 = not scheduled yet
uint32_t g_restartAtMs = 0;

class Lock {
 public:
  Lock() { xSemaphoreTake(g_mutex, portMAX_DELAY); }
  ~Lock() { xSemaphoreGive(g_mutex); }
};

void setState(State s, const String& err = "") {
  Lock l;
  g_state = s;
  g_error = err;
}

void fail(const String& err) {
  Serial.printf("[ota] error: %s\n", err.c_str());
  setState(State::Error, err);
}

// GET with certificate check and GitHub's redirect to its asset CDN.
bool openGet(HTTPClient& http, NetworkClientSecure& client, const String& url, String& err) {
  client.useBuiltinCACertBundle();
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  http.setTimeout(15000);
  http.setUserAgent("TerraLight/" TL_FW_VERSION);
  if (!http.begin(client, url)) {
    err = "cannot open " + url;
    return false;
  }
  const int code = http.GET();
  if (code == HTTP_CODE_OK) return true;
  err = code == HTTP_CODE_NOT_FOUND ? String("no release found") : code < 0 ? http.errorToString(code) : "HTTP " + String(code);
  http.end();
  return false;
}

bool isHex64(const String& s) {
  if (s.length() != 64) return false;
  for (char c : s) {
    if (!isxdigit((unsigned char)c)) return false;
  }
  return true;
}

void runCheck() {
  NetworkClientSecure client;
  HTTPClient http;
  String err;
  setState(State::Checking);
  Serial.println("[ota] checking for updates");
  if (!openGet(http, client, String(BASE_URL) + "latest/download/manifest.json", err)) return fail(err);
  const String body = http.getString();
  http.end();

  JsonDocument doc;
  if (deserializeJson(doc, body)) return fail("invalid manifest");
  const String version = doc["version"] | "";
  const String tag = doc["tag"] | "";
  const String sha = doc["sha256"] | "";
  const uint32_t size = doc["size"] | 0;
  const esp_partition_t* slot = esp_ota_get_next_update_partition(nullptr);
  if (version.isEmpty() || tag.isEmpty() || !isHex64(sha) || size == 0) return fail("incomplete manifest");
  if (!slot || size > slot->size) return fail("firmware too large for the OTA slot");

  const bool newer = compareVersions(version.c_str(), TL_FW_VERSION) > 0;
  {
    Lock l;
    g_latest = version;
    g_tag = tag;
    g_sha = sha;
    g_size = size;
  }
  Serial.printf("[ota] latest %s, running %s → %s\n", version.c_str(), TL_FW_VERSION, newer ? "update available" : "up to date");
  setState(newer ? State::Available : State::UpToDate);
}

void runInstall() {
  String tag, sha;
  uint32_t size;
  {
    Lock l;
    tag = g_tag;
    sha = g_sha;
    size = g_size;
  }
  g_progress = 0;
  setState(State::Downloading);
  Serial.printf("[ota] downloading %s (%lu B)\n", tag.c_str(), (unsigned long)size);

  NetworkClientSecure client;
  HTTPClient http;
  String err;
  if (!openGet(http, client, String(BASE_URL) + "download/" + tag + "/firmware.bin", err)) return fail(err);
  if (http.getSize() != (int)size) {
    http.end();
    return fail("size differs from manifest");
  }
  if (!Update.begin(size, U_FLASH)) {
    http.end();
    return fail(String("Update.begin: ") + Update.errorString());
  }

  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_starts(&ctx, 0);
  static uint8_t buf[2048];  // static: keep it off the task stack
  NetworkClient* stream = http.getStreamPtr();
  uint32_t done = 0;
  uint32_t lastDataMs = millis();
  bool ok = true;
  while (done < size) {
    const size_t avail = stream->available();
    if (avail) {
      const int n = stream->readBytes(buf, min(avail, sizeof(buf)));
      if (n <= 0) continue;
      mbedtls_sha256_update(&ctx, buf, n);
      if (Update.write(buf, n) != (size_t)n) {
        err = String("flash write: ") + Update.errorString();
        ok = false;
        break;
      }
      done += n;
      g_progress = (uint8_t)((uint64_t)done * 100 / size);
      lastDataMs = millis();
    } else if (millis() - lastDataMs > STALL_TIMEOUT_MS) {
      err = "download stalled";
      ok = false;
      break;
    } else {
      vTaskDelay(pdMS_TO_TICKS(5));  // let loop() and the LEDs run
    }
  }
  http.end();

  uint8_t digest[32];
  mbedtls_sha256_finish(&ctx, digest);
  mbedtls_sha256_free(&ctx);
  if (ok) {
    char hex[65];
    for (int i = 0; i < 32; i++) sprintf(hex + i * 2, "%02x", digest[i]);
    if (!sha.equalsIgnoreCase(hex)) {
      err = "SHA-256 mismatch";
      ok = false;
    }
  }
  if (!ok) {
    Update.abort();
    return fail(err);
  }
  if (!Update.end()) return fail(String("Update.end: ") + Update.errorString());

  g_progress = 100;
  Serial.println("[ota] update written, restarting");
  setState(State::Done);
}

void task(void* arg) {
  const Job job = (Job)(uintptr_t)arg;
  if (job == Job::Check) runCheck();
  else runInstall();
  g_busy = false;
  vTaskDelete(nullptr);
}

bool start(Job job) {
  if (g_busy) return false;
  g_busy = true;
  // Priority 1 = same as loop(): time-sliced, so the light keeps running.
  if (xTaskCreate(task, "ota", TASK_STACK, (void*)(uintptr_t)job, 1, nullptr) != pdPASS) {
    g_busy = false;
    fail("cannot start task");
    return false;
  }
  return true;
}

}  // namespace

void begin() {
  g_mutex = xSemaphoreCreateMutex();
  esp_ota_img_states_t st;
  const esp_partition_t* running = esp_ota_get_running_partition();
  if (running && esp_ota_get_state_partition(running, &st) == ESP_OK && st == ESP_OTA_IMG_PENDING_VERIFY) {
    g_pendingVerify = true;
    Serial.printf("[ota] new image %s — confirming after %lu s\n", TL_FW_VERSION, (unsigned long)(OTA_HEALTHY_MS / 1000));
  }
}

void loop(uint32_t nowMs, bool online) {
  if (g_pendingVerify && nowMs >= OTA_HEALTHY_MS) {
    g_pendingVerify = false;
    esp_ota_mark_app_valid_cancel_rollback();
    Serial.println("[ota] new image confirmed");
  }

  const State s = state();
  if (s == State::Done) {
    if (!g_restartAtMs) g_restartAtMs = nowMs + RESTART_DELAY_MS;
    if ((int32_t)(nowMs - g_restartAtMs) >= 0) ESP.restart();
    return;
  }

  // Automatic check: shortly after Wi-Fi comes up, then once a day.
  if (!online) return;
  if (!g_nextCheckMs) g_nextCheckMs = nowMs + FIRST_CHECK_DELAY_MS;
  if ((int32_t)(nowMs - g_nextCheckMs) >= 0) {
    g_nextCheckMs = nowMs + CHECK_INTERVAL_MS;
    if (s != State::Available && s != State::Downloading) requestCheck();
  }
}

bool requestCheck() { return start(Job::Check); }

bool requestInstall() {
  if (state() != State::Available) return false;
  return start(Job::Install);
}

State state() {
  Lock l;
  return g_state;
}

const char* stateName() {
  switch (state()) {
    case State::Checking: return "checking";
    case State::UpToDate: return "upToDate";
    case State::Available: return "available";
    case State::Downloading: return "downloading";
    case State::Done: return "done";
    case State::Error: return "error";
    default: return "idle";
  }
}

String latestVersion() {
  Lock l;
  return g_latest;
}

String lastError() {
  Lock l;
  return g_error;
}

uint8_t progress() { return g_progress; }
bool pendingVerify() { return g_pendingVerify; }

namespace {

struct Version {
  long part[3] = {0, 0, 0};
  bool pre = false;  // has a "-dev.N" style suffix
  long preNum = 0;
};

Version parse(const char* s) {
  Version v;
  if (*s == 'v' || *s == 'V') s++;
  for (int i = 0; i < 3; i++) {
    char* end;
    v.part[i] = strtol(s, &end, 10);
    s = end;
    if (*s != '.') break;
    s++;
  }
  if (*s == '-') {
    v.pre = true;
    const char* digits = s + strlen(s);
    while (digits > s && isdigit((unsigned char)digits[-1])) digits--;
    v.preNum = strtol(digits, nullptr, 10);
  }
  return v;
}

}  // namespace

int compareVersions(const char* a, const char* b) {
  const Version x = parse(a), y = parse(b);
  for (int i = 0; i < 3; i++) {
    if (x.part[i] != y.part[i]) return x.part[i] > y.part[i] ? 1 : -1;
  }
  if (x.pre != y.pre) return x.pre ? -1 : 1;  // a release beats its pre-releases
  if (x.preNum != y.preNum) return x.preNum > y.preNum ? 1 : -1;
  return 0;
}

}  // namespace ota
}  // namespace tl
