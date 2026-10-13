#pragma once
// ─────────────────────────────────────────────────────────────
//  Wi-Fi manager — non-blocking state machine (FIRMWARE.md "Wi-Fi i tryb AP")
//
//  • No saved network            → setup access point (AP).
//  • Saved network unavailable   → keep retrying in the background,
//                                  NEVER fall back to AP on its own.
//  • BOOT held 5 s               → forget the network, start AP.
//  • The light keeps running in every state — nothing here blocks.
// ─────────────────────────────────────────────────────────────
#include <Arduino.h>
#include <IPAddress.h>

namespace tl {
namespace wifi {

enum class State : uint8_t {
  Idle,        // radio off (no credentials, AP not started yet)
  Connecting,  // trying the saved network
  Connected,
  Waiting,     // connection lost / failed — retry after a back-off
  AccessPoint  // setup AP active (optionally testing new credentials)
};

enum class AttemptResult : uint8_t { None, Trying, Ok, Failed };

constexpr const char* AP_SSID = "TerraLight-Setup";
constexpr const char* AP_PASSWORD = "terralight";  // shared on purpose — AP only collects the home network

using ConnectedCallback = void (*)();

void begin(const char* hostname, ConnectedCallback onConnected);
void loop(uint32_t nowMs);

State state();
const char* stateName();
bool connected();
bool apActive();
bool hasCredentials();

String ssid();       // saved network
int32_t rssi();
IPAddress ip();      // STA IP, or AP IP while in AP mode

// From the setup page: try the new network while the AP stays up.
// On success the credentials are saved and the AP closes after a short delay.
void tryCredentials(const String& ssid, const String& password);
AttemptResult lastAttempt();

// BOOT long press / panel: forget the saved network and open the AP.
void forgetAndStartAp();

// Asynchronous scan. startScan() returns immediately; poll scanResults().
void startScan();
bool scanRunning();
int scanCount();  // -1 while running
String scanSsid(int i);
int32_t scanRssi(int i);
bool scanSecure(int i);

}  // namespace wifi
}  // namespace tl
