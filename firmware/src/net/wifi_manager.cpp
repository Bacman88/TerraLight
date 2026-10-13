#include "wifi_manager.h"

#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WiFi.h>

namespace tl {
namespace wifi {

namespace {

constexpr const char* NVS_NAMESPACE = "wifi";
constexpr uint32_t CONNECT_TIMEOUT_MS = 20000;
constexpr uint32_t ATTEMPT_TIMEOUT_MS = 20000;
constexpr uint32_t AP_CLOSE_AFTER_SUCCESS_MS = 30000;  // let the phone show "connected"
constexpr uint32_t RETRY_STEPS_MS[] = {5000, 10000, 30000, 60000};
constexpr uint8_t DNS_PORT = 53;

DNSServer g_dns;
String g_hostname;
ConnectedCallback g_onConnected = nullptr;

String g_ssid, g_pass;
State g_state = State::Idle;
uint32_t g_stateSinceMs = 0;
uint8_t g_retryIndex = 0;
uint32_t g_retryAtMs = 0;

// AP-mode credential test
AttemptResult g_attempt = AttemptResult::None;
String g_trySsid, g_tryPass;
uint32_t g_attemptStartMs = 0;
uint32_t g_apCloseAtMs = 0;
bool g_mdnsStarted = false;

// Some ESP32-C3 SuperMini batches have a poorly matched antenna and fail
// to connect at full TX power. Build with -DTL_WIFI_TX_POWER=WIFI_POWER_8_5dBm
// (platformio.ini) if the board sees networks but never connects.
void applyTxPower() {
#ifdef TL_WIFI_TX_POWER
  WiFi.setTxPower(TL_WIFI_TX_POWER);
#endif
}

void setState(State s) {
  g_state = s;
  g_stateSinceMs = millis();
}

void loadCredentials() {
  Preferences p;
  if (p.begin(NVS_NAMESPACE, true)) {
    g_ssid = p.getString("ssid", "");
    g_pass = p.getString("pass", "");
    p.end();
  }
}

void saveCredentials(const String& ssid, const String& pass) {
  Preferences p;
  if (p.begin(NVS_NAMESPACE, false)) {
    p.putString("ssid", ssid);
    p.putString("pass", pass);
    p.end();
  }
  g_ssid = ssid;
  g_pass = pass;
}

void clearCredentials() {
  Preferences p;
  if (p.begin(NVS_NAMESPACE, false)) {
    p.clear();
    p.end();
  }
  g_ssid = "";
  g_pass = "";
}

void startMdns() {
  if (g_mdnsStarted) MDNS.end();
  g_mdnsStarted = MDNS.begin(g_hostname.c_str());
  if (g_mdnsStarted) MDNS.addService("http", "tcp", 80);
}

void beginStation() {
  WiFi.mode(WIFI_STA);
  applyTxPower();
  WiFi.setHostname(g_hostname.c_str());
  WiFi.begin(g_ssid.c_str(), g_pass.c_str());
  setState(State::Connecting);
  Serial.printf("[wifi] connecting to \"%s\"\n", g_ssid.c_str());
}

void startAp() {
  WiFi.mode(WIFI_AP_STA);  // STA part enables scanning and credential tests
  applyTxPower();
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  g_dns.setErrorReplyCode(DNSReplyCode::NoError);
  g_dns.start(DNS_PORT, "*", WiFi.softAPIP());  // every domain → us (captive portal)
  g_attempt = AttemptResult::None;
  g_apCloseAtMs = 0;
  setState(State::AccessPoint);
  Serial.printf("[wifi] AP \"%s\" at %s\n", AP_SSID, WiFi.softAPIP().toString().c_str());
}

void stopAp() {
  g_dns.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  Serial.println("[wifi] AP closed");
}

void scheduleRetry(uint32_t nowMs) {
  const uint8_t n = sizeof(RETRY_STEPS_MS) / sizeof(RETRY_STEPS_MS[0]);
  const uint32_t delayMs = RETRY_STEPS_MS[g_retryIndex < n ? g_retryIndex : n - 1];
  if (g_retryIndex < n - 1) g_retryIndex++;
  g_retryAtMs = nowMs + delayMs;
  setState(State::Waiting);
  Serial.printf("[wifi] retry in %lu s\n", (unsigned long)(delayMs / 1000));
}

void onStationUp() {
  g_retryIndex = 0;
  setState(State::Connected);
  Serial.printf("[wifi] connected, IP %s, RSSI %d\n", WiFi.localIP().toString().c_str(), WiFi.RSSI());
  startMdns();
  if (g_onConnected) g_onConnected();
}

}  // namespace

void begin(const char* hostname, ConnectedCallback onConnected) {
  g_hostname = hostname;
  g_onConnected = onConnected;
  WiFi.persistent(false);       // we store credentials ourselves
  WiFi.setAutoReconnect(false); // reconnects are handled by the state machine
  loadCredentials();
  if (g_ssid.length() > 0) {
    beginStation();
  } else {
    Serial.println("[wifi] no saved network");
    startAp();
  }
}

void loop(uint32_t nowMs) {
  switch (g_state) {
    case State::Connecting:
      if (WiFi.status() == WL_CONNECTED) {
        onStationUp();
      } else if (nowMs - g_stateSinceMs > CONNECT_TIMEOUT_MS) {
        WiFi.disconnect();
        scheduleRetry(nowMs);
      }
      break;

    case State::Connected:
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[wifi] connection lost");
        g_retryIndex = 0;
        scheduleRetry(nowMs);
      }
      break;

    case State::Waiting:
      if ((int32_t)(nowMs - g_retryAtMs) >= 0) beginStation();
      break;

    case State::AccessPoint:
      g_dns.processNextRequest();
      if (g_attempt == AttemptResult::Trying) {
        if (WiFi.status() == WL_CONNECTED) {
          saveCredentials(g_trySsid, g_tryPass);
          g_attempt = AttemptResult::Ok;
          g_apCloseAtMs = nowMs + AP_CLOSE_AFTER_SUCCESS_MS;
          Serial.printf("[wifi] new network OK: %s, IP %s\n", g_trySsid.c_str(), WiFi.localIP().toString().c_str());
          startMdns();
          if (g_onConnected) g_onConnected();
        } else if (nowMs - g_attemptStartMs > ATTEMPT_TIMEOUT_MS) {
          g_attempt = AttemptResult::Failed;
          WiFi.disconnect(false);  // keep the AP up
          Serial.println("[wifi] new network failed");
        }
      }
      if (g_attempt == AttemptResult::Ok && (int32_t)(nowMs - g_apCloseAtMs) >= 0) {
        stopAp();
        g_retryIndex = 0;
        setState(WiFi.status() == WL_CONNECTED ? State::Connected : State::Connecting);
      }
      break;

    case State::Idle:
      break;
  }
}

State state() { return g_state; }

const char* stateName() {
  switch (g_state) {
    case State::Connecting: return "connecting";
    case State::Connected: return "connected";
    case State::Waiting: return "waiting";
    case State::AccessPoint: return "ap";
    default: return "idle";
  }
}

bool connected() { return WiFi.status() == WL_CONNECTED; }
bool apActive() { return g_state == State::AccessPoint; }
bool hasCredentials() { return g_ssid.length() > 0; }
String ssid() { return g_ssid; }
int32_t rssi() { return connected() ? WiFi.RSSI() : 0; }

IPAddress ip() {
  if (connected()) return WiFi.localIP();
  if (apActive()) return WiFi.softAPIP();
  return IPAddress();
}

void tryCredentials(const String& ssid, const String& password) {
  if (!apActive()) {
    // Changing the network from the normal panel: save and reconnect.
    saveCredentials(ssid, password);
    WiFi.disconnect();
    g_retryIndex = 0;
    beginStation();
    return;
  }
  g_trySsid = ssid;
  g_tryPass = password;
  g_attempt = AttemptResult::Trying;
  g_attemptStartMs = millis();
  WiFi.setHostname(g_hostname.c_str());
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.printf("[wifi] testing \"%s\" (AP stays up)\n", ssid.c_str());
}

AttemptResult lastAttempt() { return g_attempt; }

void forgetAndStartAp() {
  clearCredentials();
  WiFi.disconnect(true);
  if (g_mdnsStarted) {
    MDNS.end();
    g_mdnsStarted = false;
  }
  startAp();
}

void startScan() {
  if (WiFi.scanComplete() == WIFI_SCAN_RUNNING) return;
  WiFi.scanDelete();
  WiFi.scanNetworks(true /* async */);
}

bool scanRunning() { return WiFi.scanComplete() == WIFI_SCAN_RUNNING; }
int scanCount() {
  const int n = WiFi.scanComplete();
  return n == WIFI_SCAN_RUNNING ? -1 : (n < 0 ? 0 : n);
}
String scanSsid(int i) { return WiFi.SSID(i); }
int32_t scanRssi(int i) { return WiFi.RSSI(i); }
bool scanSecure(int i) { return WiFi.encryptionType(i) != WIFI_AUTH_OPEN; }

}  // namespace wifi
}  // namespace tl
