#include "clock.h"

#include <esp_sntp.h>
#include <sys/time.h>

namespace tl {
namespace clk {

namespace {

constexpr time_t MIN_VALID_EPOCH = 1735689600;  // 2025-01-01 — anything earlier = not set
constexpr const char* NTP_1 = "pool.ntp.org";
constexpr const char* NTP_2 = "time.google.com";

char g_tz[64] = "UTC0";
volatile Source g_source = Source::None;
volatile uint32_t g_lastSyncMs = 0;

void onSntpSync(struct timeval*) {
  g_source = Source::Ntp;
  g_lastSyncMs = millis();
}

void applyTz() {
  setenv("TZ", g_tz, 1);
  tzset();
}

}  // namespace

void begin(const char* posixTz) {
  setTimeZone(posixTz);
  sntp_set_time_sync_notification_cb(onSntpSync);
}

void setTimeZone(const char* posixTz) {
  if (!posixTz || !*posixTz) posixTz = "UTC0";
  strncpy(g_tz, posixTz, sizeof(g_tz) - 1);
  g_tz[sizeof(g_tz) - 1] = '\0';
  applyTz();
}

void onNetworkUp() {
  // configTzTime starts SNTP and sets TZ; it re-syncs every hour by default.
  configTzTime(g_tz, NTP_1, NTP_2);
}

void setManual(time_t epochUtc) {
  if (epochUtc < MIN_VALID_EPOCH) return;
  struct timeval tv = {.tv_sec = epochUtc, .tv_usec = 0};
  settimeofday(&tv, nullptr);
  applyTz();
  // NTP, once available, has priority and will overwrite this.
  if (g_source != Source::Ntp) g_source = Source::Manual;
  g_lastSyncMs = millis();
}

bool valid() { return g_source != Source::None && time(nullptr) >= MIN_VALID_EPOCH; }

Source source() { return g_source; }

const char* sourceName() {
  switch (g_source) {
    case Source::Ntp: return "ntp";
    case Source::Manual: return "manual";
    default: return "none";
  }
}

time_t now() { return time(nullptr); }

bool localTime(struct tm& out) {
  if (!valid()) return false;
  time_t t = time(nullptr);
  localtime_r(&t, &out);
  return true;
}

uint32_t lastSyncAgeSec() {
  if (g_source == Source::None) return 0;
  return (millis() - g_lastSyncMs) / 1000;
}

}  // namespace clk
}  // namespace tl
