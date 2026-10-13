#pragma once
// ─────────────────────────────────────────────────────────────
//  Clock: NTP + POSIX time zone + time set from the phone.
//
//  The time zone is a POSIX rule (configTzTime), so DST switches
//  automatically. Fixes the v1.0.0 / WeatherLight bug where
//  configTime(3600, 3600) gave UTC+2 all year round.
//
//  The ESP32-C3 has no battery-backed RTC: after a power loss the
//  time is unknown until NTP or the phone sets it — valid() == false.
//  Light logic must then fall back to "safe night" (M2).
// ─────────────────────────────────────────────────────────────
#include <Arduino.h>
#include <time.h>

namespace tl {
namespace clk {

enum class Source : uint8_t { None = 0, Ntp = 1, Manual = 2 };

void begin(const char* posixTz);
void setTimeZone(const char* posixTz);
void onNetworkUp();               // start/refresh NTP
void setManual(time_t epochUtc);  // time sent by the phone

bool valid();
Source source();
const char* sourceName();
time_t now();
bool localTime(struct tm& out);
uint32_t lastSyncAgeSec();  // seconds since the last NTP/manual set

}  // namespace clk
}  // namespace tl
