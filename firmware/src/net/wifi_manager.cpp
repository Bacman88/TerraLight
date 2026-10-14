#include "wifi_manager.h"

#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <WiFi.h>
#include <esp_wifi.h>

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
bool g_holdAp = false;  // first-run wizard still open on the phone
bool g_mdnsStarted = false;

// Disconnect reasons from the router arrive on the Wi-Fi event task;
// loop() logs them and keeps the one that ended a connection attempt.
volatile uint8_t g_evReason = 0;
volatile uint32_t g_evCount = 0;
uint32_t g_evSeen = 0;
uint8_t g_lastReason = 0;  // 0 = none

// Associated with the router (before DHCP) — tells "no IP" apart from "rejected".
volatile uint32_t g_evAssocCount = 0;
uint32_t g_evAssocSeen = 0;
bool g_associated = false;

void onStaEvent(arduino_event_id_t event, arduino_event_info_t info) {
  if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
    g_evReason = info.wifi_sta_disconnected.reason;
    g_evCount = g_evCount + 1;
  } else if (event == ARDUINO_EVENT_WIFI_STA_CONNECTED) {
    g_evAssocCount = g_evAssocCount + 1;
  }
}

// A running async scan makes esp_wifi_connect() fail, so stop it first.
void stopScan() {
  if (WiFi.scanComplete() == WIFI_SCAN_RUNNING) {
    esp_wifi_scan_stop();
    WiFi.scanDelete();
    Serial.println("[wifi] scan stopped before connecting");
  }
}

void startConnect(const String& ssid, const String& pass) {
  stopScan();
  g_lastReason = 0;
  g_associated = false;
  const bool ok = WiFi.begin(ssid.c_str(), pass.c_str()) != WL_CONNECT_FAILED;
  if (!ok) Serial.println("[wifi] WiFi.begin() refused");
}

// Some ESP32-C3 SuperMini batches have a poorly matched antenna and fail
// to connect at full TX power. Build with -DTL_WIFI_TX_POWER=WIFI_POWER_8_5dBm
// (platformio.ini) if the board sees networks but never connects.
void applyTxPower() {
#ifdef TL_WIFI_TX_POWER
  WiFi.setTxPower(TL_WIFI_TX_POWER);
#endif
}

// Signed difference: a timestamp taken after nowMs (e.g. set by a web request
// handled earlier in the same loop()) must count as 0, not as ~49 days.
int32_t elapsed(uint32_t nowMs, uint32_t sinceMs) { return (int32_t)(nowMs - sinceMs); }

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
  startConnect(g_ssid, g_pass);
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
  WiFi.onEvent(onStaEvent, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.onEvent(onStaEvent, ARDUINO_EVENT_WIFI_STA_CONNECTED);
  loadCredentials();
  if (g_ssid.length() > 0) {
    beginStation();
  } else {
    Serial.println("[wifi] no saved network");
    startAp();
  }
}

void loop(uint32_t nowMs) {
  if (g_evAssocCount != g_evAssocSeen) {
    g_evAssocSeen = g_evAssocCount;
    g_associated = true;
    Serial.println("[wifi] associated with the router, waiting for IP");
  }
  if (g_evCount != g_evSeen) {
    g_evSeen = g_evCount;
    const uint8_t r = g_evReason;
    Serial.printf("[wifi] disconnected, reason %u (%s)\n", r, reasonName(r));
    // Ignore our own WiFi.disconnect() after a timeout — keep the router's reason.
    if (g_state == State::Connecting || g_attempt == AttemptResult::Trying) g_lastReason = r;
  }

  switch (g_state) {
    case State::Connecting:
      if (WiFi.status() == WL_CONNECTED) {
        onStationUp();
      } else if (elapsed(nowMs, g_stateSinceMs) > (int32_t)CONNECT_TIMEOUT_MS) {
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
        } else if (elapsed(nowMs, g_attemptStartMs) > (int32_t)ATTEMPT_TIMEOUT_MS) {
          g_attempt = AttemptResult::Failed;
          WiFi.disconnect(false);  // keep the AP up
          if (g_associated && !g_lastReason) Serial.println("[wifi] new network failed: associated but no IP (DHCP)");
          else Serial.printf("[wifi] new network failed, reason %u (%s)\n", g_lastReason, reasonName(g_lastReason));
        }
      }
      // Keep the AP while the wizard runs; close it a while after it finishes.
      if (g_attempt == AttemptResult::Ok && g_holdAp) g_apCloseAtMs = nowMs + AP_CLOSE_AFTER_SUCCESS_MS;
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
  startConnect(ssid, password);
  Serial.printf("[wifi] testing \"%s\" (AP stays up)\n", ssid.c_str());
}

AttemptResult lastAttempt() { return g_attempt; }

void holdAp(bool hold) { g_holdAp = hold; }

uint8_t lastReason() { return g_lastReason; }

const char* reasonName(uint8_t reason) {
  return reason ? WiFi.disconnectReasonName((wifi_err_reason_t)reason) : "none";
}

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
