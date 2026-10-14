#include "server.h"

#include <ArduinoJson.h>
#include <WebServer.h>
#include <WiFi.h>

#include "../core/clock.h"
#include "../core/settings.h"
#include "../core/version.h"
#include "../light/led_output.h"
#include "../net/wifi_manager.h"
#include "api_json.h"
#include "assets.h"

namespace tl {
namespace web {

namespace {

WebServer g_server(80);

// Actions that must run after the HTTP response has been sent.
enum class Pending : uint8_t { None, Restart, ForgetWifi };
Pending g_pending = Pending::None;
uint32_t g_pendingAtMs = 0;

void schedule(Pending p, uint32_t delayMs = 600) {
  g_pending = p;
  g_pendingAtMs = millis() + delayMs;
}

void sendJson(int code, const JsonDocument& doc) {
  String body;
  serializeJson(doc, body);
  g_server.sendHeader("Cache-Control", "no-store");
  g_server.send(code, "application/json", body);
}

void sendError(int code, const String& message) {
  JsonDocument doc;
  doc["ok"] = false;
  doc["error"] = message;
  sendJson(code, doc);
}

bool readBody(JsonDocument& doc) {
  if (!g_server.hasArg("plain")) {
    sendError(400, "missing JSON body");
    return false;
  }
  const DeserializationError err = deserializeJson(doc, g_server.arg("plain"));
  if (err) {
    sendError(400, String("invalid JSON: ") + err.c_str());
    return false;
  }
  return true;
}

// ── Captive portal ───────────────────────────────────────────
// While the setup AP is active, any request for a foreign host (phone
// connectivity checks, typed addresses) is redirected to our page,
// which makes phones open the setup screen automatically.
bool captiveRedirect() {
  if (!wifi::apActive()) return false;
  const String host = g_server.hostHeader();
  const String apIp = WiFi.softAPIP().toString();
  if (host == apIp || host.startsWith(apIp + ":")) return false;
  const String hostname = String(settings::get().system.hostname) + ".local";
  if (host.equalsIgnoreCase(hostname)) return false;
  g_server.sendHeader("Location", String("http://") + apIp + "/", true);
  g_server.send(302, "text/plain", "");
  return true;
}

// ── Panel ────────────────────────────────────────────────────
void handleRoot() {
  if (captiveRedirect()) return;
  g_server.sendHeader("Content-Encoding", "gzip");
  g_server.sendHeader("Cache-Control", "no-cache");
  g_server.send_P(200, "text/html; charset=utf-8", reinterpret_cast<const char*>(PANEL_HTML_GZ),
                  PANEL_HTML_GZ_LEN);
}

void handleNotFound() {
  if (captiveRedirect()) return;
  if (g_server.uri().startsWith("/api/")) {
    sendError(404, "unknown endpoint");
    return;
  }
  // Unknown page → panel (single-page app)
  g_server.sendHeader("Location", "/", true);
  g_server.send(302, "text/plain", "");
}

// ── /api/status ──────────────────────────────────────────────
void handleStatus() {
  JsonDocument doc;
  doc["ok"] = true;
  doc["name"] = TL_FW_NAME;
  doc["version"] = TL_FW_VERSION;
  doc["uptimeSec"] = millis() / 1000;
  doc["heapFree"] = ESP.getFreeHeap();
  doc["profile"] = settings::get().profile == Profile::Aqua ? "aqua" : "terra";

  JsonObject t = doc["time"].to<JsonObject>();
  t["valid"] = clk::valid();
  t["source"] = clk::sourceName();
  t["tzName"] = settings::get().time.tzName;
  if (clk::valid()) {
    t["epoch"] = (uint32_t)clk::now();
    struct tm lt;
    if (clk::localTime(lt)) {
      char buf[24];
      strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &lt);
      t["local"] = buf;
    }
    t["syncAgeSec"] = clk::lastSyncAgeSec();
  }

  JsonObject w = doc["wifi"].to<JsonObject>();
  w["state"] = wifi::stateName();
  w["ssid"] = wifi::ssid();
  w["ip"] = wifi::ip().toString();
  w["rssi"] = wifi::rssi();
  w["hostname"] = String(settings::get().system.hostname) + ".local";
  w["apActive"] = wifi::apActive();
  w["apSsid"] = wifi::AP_SSID;
  switch (wifi::lastAttempt()) {
    case wifi::AttemptResult::Trying: w["attempt"] = "trying"; break;
    case wifi::AttemptResult::Ok: w["attempt"] = "ok"; break;
    case wifi::AttemptResult::Failed: w["attempt"] = "failed"; break;
    default: w["attempt"] = "none"; break;
  }
  w["reason"] = wifi::lastReason();
  w["reasonName"] = wifi::reasonName(wifi::lastReason());

  const led::Stats ls = led::stats();
  JsonObject l = doc["led"].to<JsonObject>();
  l["count"] = ls.count;
  l["estimatedMa"] = ls.estimatedMa;
  l["requestedMa"] = ls.requestedMa;
  l["limited"] = ls.limiterScale < 0.999f;
  l["limiterScale"] = serialized(String(ls.limiterScale, 2));
  l["softStart"] = serialized(String(ls.softStart, 2));

  sendJson(200, doc);
}

// ── /api/config ──────────────────────────────────────────────
void sendConfig(bool restartRequired = false) {
  JsonDocument doc;
  doc["ok"] = true;
  api::settingsToJson(settings::get(), doc["config"].to<JsonObject>());
  if (restartRequired) doc["restartRequired"] = true;
  sendJson(200, doc);
}

void handleConfigGet() { sendConfig(); }

void handleConfigPost() {
  JsonDocument in;
  if (!readBody(in)) return;
  Settings& s = settings::get();
  const api::ApplyResult r = api::applyJson(in.as<JsonObjectConst>(), s);
  if (r.error.length()) {
    sendError(400, r.error);
    return;
  }
  if (r.changed) settings::markDirty();
  if (r.timeZoneChanged) clk::setTimeZone(s.time.tz);
  wifi::holdAp(!s.system.setupDone);
  sendConfig(r.restartRequired);
}

// ── /api/time ────────────────────────────────────────────────
void handleTimePost() {
  JsonDocument in;
  if (!readBody(in)) return;
  if (in["epoch"].is<uint32_t>()) clk::setManual((time_t)in["epoch"].as<uint32_t>());
  JsonDocument out;
  out["ok"] = clk::valid();
  out["source"] = clk::sourceName();
  sendJson(clk::valid() ? 200 : 400, out);
}

// ── /api/wifi ────────────────────────────────────────────────
void handleWifiScan() {
  JsonDocument doc;
  const int n = wifi::scanCount();
  if (n < 0) {
    doc["ok"] = true;
    doc["running"] = true;
    sendJson(200, doc);
    return;
  }
  if (n == 0 || g_server.hasArg("refresh")) {
    wifi::startScan();
    doc["ok"] = true;
    doc["running"] = true;
    sendJson(200, doc);
    return;
  }
  doc["ok"] = true;
  doc["running"] = false;
  JsonArray arr = doc["networks"].to<JsonArray>();
  for (int i = 0; i < n && i < 30; i++) {
    const String ssid = wifi::scanSsid(i);
    if (ssid.isEmpty()) continue;
    JsonObject net = arr.add<JsonObject>();
    net["ssid"] = ssid;
    net["rssi"] = wifi::scanRssi(i);
    net["secure"] = wifi::scanSecure(i);
  }
  sendJson(200, doc);
}

void handleWifiPost() {
  JsonDocument in;
  if (!readBody(in)) return;
  const String ssid = in["ssid"] | "";
  const String pass = in["password"] | "";
  if (ssid.isEmpty() || ssid.length() > 32 || pass.length() > 64) {
    sendError(400, "invalid SSID or password");
    return;
  }
  wifi::tryCredentials(ssid, pass);
  JsonDocument out;
  out["ok"] = true;
  out["testing"] = wifi::apActive();
  sendJson(200, out);
}

void handleWifiForget() {
  JsonDocument out;
  out["ok"] = true;
  sendJson(200, out);
  schedule(Pending::ForgetWifi);
}

// ── /api/system ──────────────────────────────────────────────
void handleRestart() {
  settings::saveNow();
  JsonDocument out;
  out["ok"] = true;
  sendJson(200, out);
  schedule(Pending::Restart);
}

void handleDefaults() {
  JsonDocument in;
  if (!readBody(in)) return;
  const String p = in["profile"] | "terra";
  settings::resetToDefaults(p == "aqua" ? Profile::Aqua : Profile::Terra);
  settings::markDirty();
  settings::saveNow();
  clk::setTimeZone(settings::get().time.tz);
  wifi::holdAp(!settings::get().system.setupDone);
  sendConfig();
}

}  // namespace

void begin() {
  g_server.on("/", HTTP_GET, handleRoot);
  g_server.on("/api/status", HTTP_GET, handleStatus);
  g_server.on("/api/config", HTTP_GET, handleConfigGet);
  g_server.on("/api/config", HTTP_POST, handleConfigPost);
  g_server.on("/api/time", HTTP_POST, handleTimePost);
  g_server.on("/api/wifi/scan", HTTP_GET, handleWifiScan);
  g_server.on("/api/wifi", HTTP_POST, handleWifiPost);
  g_server.on("/api/wifi/forget", HTTP_POST, handleWifiForget);
  g_server.on("/api/system/restart", HTTP_POST, handleRestart);
  g_server.on("/api/system/defaults", HTTP_POST, handleDefaults);
  // Phone / OS connectivity checks → panel while in AP
  g_server.on("/generate_204", handleRoot);         // Android
  g_server.on("/hotspot-detect.html", handleRoot);  // Apple
  g_server.on("/connecttest.txt", handleRoot);      // Windows
  g_server.on("/ncsi.txt", handleRoot);             // Windows (older)
  g_server.onNotFound(handleNotFound);

  const char* headers[] = {"Host"};
  g_server.collectHeaders(headers, 1);
  g_server.begin();
  Serial.println("[web] server on port 80");
}

void loop() {
  g_server.handleClient();

  if (g_pending != Pending::None && (int32_t)(millis() - g_pendingAtMs) >= 0) {
    const Pending p = g_pending;
    g_pending = Pending::None;
    if (p == Pending::Restart) {
      ESP.restart();
    } else if (p == Pending::ForgetWifi) {
      wifi::forgetAndStartAp();
    }
  }
}

}  // namespace web
}  // namespace tl
