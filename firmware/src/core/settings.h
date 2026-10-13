#pragma once
// ─────────────────────────────────────────────────────────────
//  Persistent settings (NVS)
//
//  One struct stored as a single blob with a schema number.
//  Bump SETTINGS_SCHEMA when the layout changes and add a migration
//  in settings.cpp — an OTA update must never wipe user settings.
//  Wi-Fi credentials live separately (net/wifi_manager).
// ─────────────────────────────────────────────────────────────
#include <Arduino.h>

namespace tl {

constexpr uint16_t SETTINGS_SCHEMA = 1;
constexpr uint16_t MAX_LEDS = 300;

enum class Profile : uint8_t { Terra = 0, Aqua = 1 };
enum class ModulePreset : uint8_t { Ring8 = 0, Ring12 = 1, Arc = 2, Line = 3, Custom = 4 };
enum class LedChip : uint8_t { WS2812B = 0, WS2811 = 1 };
enum class ColorOrder : uint8_t { RGB = 0, GRB = 1, BRG = 2, RBG = 3, GBR = 4, BGR = 5 };
enum class Shape : uint8_t { Line = 0, Ring = 1, Arc = 2 };

struct ModuleSettings {
  ModulePreset preset;
  LedChip chip;
  ColorOrder order;
  Shape shape;
  uint16_t count;       // number of pixels
  uint16_t maPerPixel;  // worst-case current of one pixel at full white
  uint16_t budgetMa;    // current available for LEDs (supply minus ESP32 margin)
};

struct LightSettings {
  bool on;
  uint8_t r, g, b;     // static colour
  uint8_t brightness;  // 0–100 %
  uint8_t density;     // 1 = every pixel, 2 = every second, …
};

struct OutputSettings {
  uint8_t maxBrightness;  // global limit 1–100 %
  uint16_t softStartSec;  // fade-in after power-up
};

struct TimeSettings {
  char tz[64];      // POSIX TZ rule, e.g. "CET-1CEST,M3.5.0,M10.5.0/3"
  char tzName[48];  // display name, e.g. "Europe/Warsaw"
};

struct SystemSettings {
  char hostname[32];  // mDNS: http://<hostname>.local
  char lang[4];       // "pl" / "en"
};

struct Settings {
  Profile profile;
  ModuleSettings module;
  LightSettings light;
  OutputSettings output;
  TimeSettings time;
  SystemSettings system;
};

namespace settings {

void begin();
Settings& get();

// Call after changing anything in get(). Saving is deferred by ~2 s so
// dragging a slider in the panel does not wear out the flash.
void markDirty();
void loop(uint32_t nowMs);
void saveNow();

// Increments on every change; modules compare it to react to changes.
uint32_t revision();

void resetToDefaults(Profile profile);
void applyModulePreset(ModuleSettings& m, ModulePreset preset);

}  // namespace settings
}  // namespace tl
