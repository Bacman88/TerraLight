#include "api_json.h"

namespace tl {
namespace api {

namespace {

template <typename E>
struct EnumName {
  E value;
  const char* name;
};

constexpr EnumName<Profile> PROFILES[] = {{Profile::Terra, "terra"}, {Profile::Aqua, "aqua"}};
constexpr EnumName<ModulePreset> PRESETS[] = {{ModulePreset::Ring8, "ring8"},
                                              {ModulePreset::Ring12, "ring12"},
                                              {ModulePreset::Arc, "arc"},
                                              {ModulePreset::Line, "line"},
                                              {ModulePreset::Custom, "custom"}};
constexpr EnumName<LedChip> CHIPS[] = {{LedChip::WS2812B, "ws2812b"}, {LedChip::WS2811, "ws2811"}};
constexpr EnumName<ColorOrder> ORDERS[] = {{ColorOrder::RGB, "rgb"}, {ColorOrder::GRB, "grb"},
                                           {ColorOrder::BRG, "brg"}, {ColorOrder::RBG, "rbg"},
                                           {ColorOrder::GBR, "gbr"}, {ColorOrder::BGR, "bgr"}};
constexpr EnumName<Shape> SHAPES[] = {{Shape::Line, "line"}, {Shape::Ring, "ring"}, {Shape::Arc, "arc"}};

template <typename E, size_t N>
const char* toName(const EnumName<E> (&table)[N], E v) {
  for (const auto& e : table)
    if (e.value == v) return e.name;
  return table[0].name;
}

template <typename E, size_t N>
bool fromName(const EnumName<E> (&table)[N], const char* name, E& out) {
  if (!name) return false;
  for (const auto& e : table)
    if (strcasecmp(e.name, name) == 0) {
      out = e.value;
      return true;
    }
  return false;
}

String colorToHex(uint8_t r, uint8_t g, uint8_t b) {
  char buf[8];
  snprintf(buf, sizeof(buf), "#%02x%02x%02x", r, g, b);
  return String(buf);
}

bool hexToColor(const char* hex, uint8_t& r, uint8_t& g, uint8_t& b) {
  if (!hex) return false;
  if (*hex == '#') hex++;
  if (strlen(hex) != 6) return false;
  char* end = nullptr;
  const unsigned long v = strtoul(hex, &end, 16);
  if (!end || *end != '\0') return false;
  r = (v >> 16) & 0xFF;
  g = (v >> 8) & 0xFF;
  b = v & 0xFF;
  return true;
}

bool copyIfPresent(JsonVariantConst v, char* dst, size_t size) {
  if (!v.is<const char*>()) return false;
  const char* src = v.as<const char*>();
  if (strcmp(dst, src) == 0) return false;
  strncpy(dst, src, size - 1);
  dst[size - 1] = '\0';
  return true;
}

template <typename T>
bool setInt(JsonVariantConst v, T& dst, long minV, long maxV) {
  if (!v.is<long>()) return false;
  const long n = constrain(v.as<long>(), minV, maxV);
  if ((long)dst == n) return false;
  dst = (T)n;
  return true;
}

bool validHostname(const char* h) {
  const size_t n = strlen(h);
  if (n == 0 || n > 31) return false;
  for (size_t i = 0; i < n; i++) {
    const char c = h[i];
    if (!(isalnum((unsigned char)c) || c == '-')) return false;
  }
  return h[0] != '-' && h[n - 1] != '-';
}

}  // namespace

void settingsToJson(const Settings& s, JsonObject out) {
  out["profile"] = toName(PROFILES, s.profile);

  JsonObject m = out["module"].to<JsonObject>();
  m["preset"] = toName(PRESETS, s.module.preset);
  m["chip"] = toName(CHIPS, s.module.chip);
  m["order"] = toName(ORDERS, s.module.order);
  m["shape"] = toName(SHAPES, s.module.shape);
  m["count"] = s.module.count;
  m["maPerPixel"] = s.module.maPerPixel;
  m["budgetMa"] = s.module.budgetMa;
  m["maxCount"] = MAX_LEDS;

  JsonObject l = out["light"].to<JsonObject>();
  l["on"] = s.light.on;
  l["color"] = colorToHex(s.light.r, s.light.g, s.light.b);
  l["brightness"] = s.light.brightness;
  l["density"] = s.light.density;

  JsonObject o = out["output"].to<JsonObject>();
  o["maxBrightness"] = s.output.maxBrightness;
  o["softStartSec"] = s.output.softStartSec;

  JsonObject t = out["time"].to<JsonObject>();
  t["tz"] = s.time.tz;
  t["tzName"] = s.time.tzName;

  JsonObject sys = out["system"].to<JsonObject>();
  sys["hostname"] = s.system.hostname;
  sys["lang"] = s.system.lang;
  sys["setupDone"] = s.system.setupDone;
}

ApplyResult applyJson(JsonObjectConst in, Settings& s) {
  ApplyResult r;

  if (in["profile"].is<const char*>()) {
    Profile p;
    if (fromName(PROFILES, in["profile"], p) && p != s.profile) {
      s.profile = p;
      r.changed = true;
    }
  }

  JsonObjectConst m = in["module"];
  if (!m.isNull()) {
    ModuleSettings before = s.module;
    // A preset first sets its defaults; explicit fields below override them.
    ModulePreset preset;
    if (fromName(PRESETS, m["preset"], preset) && preset != s.module.preset) settings::applyModulePreset(s.module, preset);
    LedChip chip;
    if (fromName(CHIPS, m["chip"], chip)) s.module.chip = chip;
    ColorOrder order;
    if (fromName(ORDERS, m["order"], order)) s.module.order = order;
    Shape shape;
    if (fromName(SHAPES, m["shape"], shape)) s.module.shape = shape;
    setInt(m["count"], s.module.count, 1, MAX_LEDS);
    setInt(m["maPerPixel"], s.module.maPerPixel, 1, 120);
    setInt(m["budgetMa"], s.module.budgetMa, 100, 10000);
    if (memcmp(&before, &s.module, sizeof(before)) != 0) {
      r.changed = r.moduleChanged = true;
    }
  }

  JsonObjectConst l = in["light"];
  if (!l.isNull()) {
    if (l["on"].is<bool>() && l["on"].as<bool>() != s.light.on) {
      s.light.on = l["on"].as<bool>();
      r.changed = true;
    }
    uint8_t cr, cg, cb;
    if (hexToColor(l["color"], cr, cg, cb) && (cr != s.light.r || cg != s.light.g || cb != s.light.b)) {
      s.light.r = cr;
      s.light.g = cg;
      s.light.b = cb;
      r.changed = true;
    }
    r.changed |= setInt(l["brightness"], s.light.brightness, 0, 100);
    r.changed |= setInt(l["density"], s.light.density, 1, 10);
  }

  JsonObjectConst o = in["output"];
  if (!o.isNull()) {
    r.changed |= setInt(o["maxBrightness"], s.output.maxBrightness, 1, 100);
    r.changed |= setInt(o["softStartSec"], s.output.softStartSec, 0, 600);
  }

  JsonObjectConst t = in["time"];
  if (!t.isNull()) {
    if (copyIfPresent(t["tz"], s.time.tz, sizeof(s.time.tz))) r.changed = r.timeZoneChanged = true;
    r.changed |= copyIfPresent(t["tzName"], s.time.tzName, sizeof(s.time.tzName));
  }

  JsonObjectConst sys = in["system"];
  if (!sys.isNull()) {
    if (sys["hostname"].is<const char*>()) {
      const char* h = sys["hostname"];
      if (!validHostname(h)) {
        r.error = "hostname: letters, digits and '-' only (max 31)";
      } else if (copyIfPresent(sys["hostname"], s.system.hostname, sizeof(s.system.hostname))) {
        r.changed = r.restartRequired = true;
      }
    }
    if (sys["lang"].is<const char*>()) {
      const char* lang = sys["lang"];
      if (strcmp(lang, "pl") == 0 || strcmp(lang, "en") == 0) {
        r.changed |= copyIfPresent(sys["lang"], s.system.lang, sizeof(s.system.lang));
      }
    }
    if (sys["setupDone"].is<bool>()) {
      const bool done = sys["setupDone"];
      if (done != s.system.setupDone) {
        s.system.setupDone = done;
        r.changed = true;
      }
    }
  }

  return r;
}

}  // namespace api
}  // namespace tl
