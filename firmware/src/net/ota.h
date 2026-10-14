#pragma once
// ─────────────────────────────────────────────────────────────
//  OTA updates from GitHub Releases — no token
//
//  Every release carries two assets:
//    manifest.json  {"version","tag","size","sha256"}
//    firmware.bin
//  The device reads .../releases/latest/download/manifest.json
//  (a public redirect, not the rate-limited API), compares the
//  version and, on request, downloads .../download/<tag>/firmware.bin
//  into the other OTA slot, checking size and SHA-256.
//
//  • HTTPS with the core's built-in CA bundle (certificate checked).
//  • Network work runs in its own FreeRTOS task — loop() never blocks.
//  • Rollback: a new image must run OTA_HEALTHY_MS before it is marked
//    valid; a crash or reset before that boots the previous version.
// ─────────────────────────────────────────────────────────────
#include <Arduino.h>

namespace tl {
namespace ota {

enum class State : uint8_t { Idle, Checking, UpToDate, Available, Downloading, Done, Error };

void begin();
void loop(uint32_t nowMs, bool online);

// Both return false if a check or download is already running.
bool requestCheck();
bool requestInstall();  // only in State::Available

State state();
const char* stateName();
String latestVersion();
String lastError();
uint8_t progress();       // 0–100 while downloading
bool pendingVerify();     // running a fresh OTA image not yet marked valid

// Semantic version compare: "2.0.0" > "2.0.0-dev.3" > "2.0.0-dev.2".
int compareVersions(const char* a, const char* b);

}  // namespace ota
}  // namespace tl
