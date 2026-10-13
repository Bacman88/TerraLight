#include "settings.h"

#include <Preferences.h>

namespace tl {
namespace settings {

namespace {

constexpr const char* NVS_NAMESPACE = "tl";
constexpr const char* NVS_KEY_BLOB = "cfg";
constexpr const char* NVS_KEY_SCHEMA = "schema";
constexpr uint32_t SAVE_DELAY_MS = 2000;

Settings g_settings;
uint32_t g_revision = 1;
bool g_dirty = false;
uint32_t g_dirtySinceMs = 0;

void copyStr(char* dst, size_t size, const char* src) {
  strncpy(dst, src, size - 1);
  dst[size - 1] = '\0';
}

}  // namespace

void applyModulePreset(ModuleSettings& m, ModulePreset preset) {
  m.preset = preset;
  switch (preset) {
    case ModulePreset::Ring8:
      m.chip = LedChip::WS2812B;
      m.shape = Shape::Ring;
      m.count = 8;
      m.maPerPixel = 60;
      break;
    case ModulePreset::Ring12:
      m.chip = LedChip::WS2812B;
      m.shape = Shape::Ring;
      m.count = 12;
      m.maPerPixel = 60;
      break;
    case ModulePreset::Arc:
      // GFLAI-style WS2811 fairy pixels on a flexible wire
      m.chip = LedChip::WS2811;
      m.shape = Shape::Arc;
      m.count = 20;
      m.maPerPixel = 20;
      break;
    case ModulePreset::Line:
      m.chip = LedChip::WS2812B;
      m.shape = Shape::Line;
      m.count = 30;
      m.maPerPixel = 60;
      break;
    case ModulePreset::Custom:
      break;  // keep the user's values
  }
  // Colour order: RGB by default until verified on samples (FIRMWARE.md).
  m.order = ColorOrder::RGB;
}

void resetToDefaults(Profile profile) {
  Settings& s = g_settings;
  memset(&s, 0, sizeof(s));

  s.profile = profile;

  applyModulePreset(s.module, ModulePreset::Ring12);
  s.module.budgetMa = 1500;  // 5 V / 2 A supply, ~500 mA left for ESP32 + Wi-Fi

  s.light.on = true;
  s.light.r = 255;
  s.light.g = 190;
  s.light.b = 120;  // warm white
  s.light.brightness = 60;
  s.light.density = 1;

  s.output.maxBrightness = 100;
  s.output.softStartSec = 60;

  copyStr(s.time.tz, sizeof(s.time.tz), "CET-1CEST,M3.5.0,M10.5.0/3");
  copyStr(s.time.tzName, sizeof(s.time.tzName), "Europe/Warsaw");

  copyStr(s.system.hostname, sizeof(s.system.hostname), "terralight");
  copyStr(s.system.lang, sizeof(s.system.lang), "pl");

  if (profile == Profile::Aqua) {
    s.light.r = 220;
    s.light.g = 235;
    s.light.b = 255;  // cool white for water
  }
}

void begin() {
  resetToDefaults(Profile::Terra);

  Preferences prefs;
  if (!prefs.begin(NVS_NAMESPACE, true)) {
    Serial.println("[settings] no saved settings — defaults");
    return;
  }

  const uint16_t schema = prefs.getUShort(NVS_KEY_SCHEMA, 0);
  const size_t len = prefs.getBytesLength(NVS_KEY_BLOB);

  if (schema == SETTINGS_SCHEMA && len == sizeof(Settings)) {
    Settings loaded;
    prefs.getBytes(NVS_KEY_BLOB, &loaded, sizeof(loaded));
    g_settings = loaded;
    // Guard against corrupted values.
    if (g_settings.module.count == 0 || g_settings.module.count > MAX_LEDS) g_settings.module.count = 12;
    if (g_settings.light.density == 0) g_settings.light.density = 1;
    if (g_settings.output.maxBrightness == 0) g_settings.output.maxBrightness = 100;
    g_settings.time.tz[sizeof(g_settings.time.tz) - 1] = '\0';
    g_settings.time.tzName[sizeof(g_settings.time.tzName) - 1] = '\0';
    g_settings.system.hostname[sizeof(g_settings.system.hostname) - 1] = '\0';
    g_settings.system.lang[sizeof(g_settings.system.lang) - 1] = '\0';
    Serial.printf("[settings] loaded (schema %u)\n", schema);
  } else if (schema != 0) {
    // Future: migrations from older schemas go here.
    Serial.printf("[settings] schema %u not supported (expected %u) — defaults\n", schema, SETTINGS_SCHEMA);
  }
  prefs.end();
}

Settings& get() { return g_settings; }

uint32_t revision() { return g_revision; }

void markDirty() {
  g_revision++;
  g_dirtySinceMs = millis();  // save 2 s after the LAST change
  g_dirty = true;
}

void saveNow() {
  Preferences prefs;
  if (!prefs.begin(NVS_NAMESPACE, false)) {
    Serial.println("[settings] NVS open failed");
    return;
  }
  prefs.putUShort(NVS_KEY_SCHEMA, SETTINGS_SCHEMA);
  prefs.putBytes(NVS_KEY_BLOB, &g_settings, sizeof(g_settings));
  prefs.end();
  g_dirty = false;
  Serial.println("[settings] saved");
}

void loop(uint32_t nowMs) {
  if (g_dirty && nowMs - g_dirtySinceMs >= SAVE_DELAY_MS) saveNow();
}

}  // namespace settings
}  // namespace tl
