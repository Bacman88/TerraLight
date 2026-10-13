#include "led_output.h"

#include <Adafruit_NeoPixel.h>
#include <math.h>

#include "../core/pins.h"
#include "../core/settings.h"

namespace tl {
namespace led {

namespace {

constexpr float GAMMA = 2.2f;
constexpr uint32_t TRANSITION_MS = 1500;
constexpr float TRANSITION_TAU_MS = 350.0f;
constexpr float QUIESCENT_MA_PER_PIXEL = 1.0f;  // WS28xx idle draw

Adafruit_NeoPixel g_strip(0, pins::LED_DATA, NEO_RGB + NEO_KHZ800);

RGBf g_target[MAX_LEDS];
RGBf g_current[MAX_LEDS];
FrameBuffer g_targetFb{g_target, 0};

uint32_t g_appliedRevision = 0;
uint16_t g_count = 0;
neoPixelType g_type = 0;
uint32_t g_firstShowMs = 0;
bool g_firstShow = true;
uint32_t g_transitionUntilMs = 0;
uint8_t g_pwm[MAX_LEDS][3];

Stats g_stats{};

neoPixelType neoType(const ModuleSettings& m) {
  neoPixelType order;
  switch (m.order) {
    case ColorOrder::GRB: order = NEO_GRB; break;
    case ColorOrder::BRG: order = NEO_BRG; break;
    case ColorOrder::RBG: order = NEO_RBG; break;
    case ColorOrder::GBR: order = NEO_GBR; break;
    case ColorOrder::BGR: order = NEO_BGR; break;
    case ColorOrder::RGB:
    default: order = NEO_RGB; break;
  }
  // WS2811 fairy pixels sold today run at 800 kHz like the WS2812B.
  return order + NEO_KHZ800;
}

void applyModuleSettings() {
  const ModuleSettings& m = settings::get().module;
  const uint16_t newCount = constrain(m.count, (uint16_t)1, MAX_LEDS);

  // Blank the old length first, so pixels beyond a shorter new length go dark.
  if (g_count > 0) {
    g_strip.clear();
    g_strip.show();
  }

  g_type = neoType(m);
  g_strip.updateType(g_type);
  g_strip.updateLength(newCount);
  g_strip.clear();
  g_strip.show();

  if (newCount != g_count) {
    for (uint16_t i = 0; i < MAX_LEDS; i++) g_current[i] = RGBf{};
  }
  g_count = newCount;
  g_targetFb.count = newCount;
  Serial.printf("[led] %u px, order %u, %u mA/px, budget %u mA\n", newCount, (unsigned)m.order, m.maPerPixel,
                m.budgetMa);
}

inline float clamp01(float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); }

inline float smoothstep(float x) {
  x = clamp01(x);
  return x * x * (3.0f - 2.0f * x);
}

}  // namespace

void begin() {
  g_strip.begin();
  applyModuleSettings();
  g_appliedRevision = settings::revision();
}

void loop(uint32_t) {
  if (settings::revision() == g_appliedRevision) return;
  g_appliedRevision = settings::revision();

  const ModuleSettings& m = settings::get().module;
  const neoPixelType wantedType = neoType(m);
  const uint16_t wantedCount = constrain(m.count, (uint16_t)1, MAX_LEDS);
  if (wantedCount != g_count || wantedType != g_type) applyModuleSettings();
  kickTransition();
}

FrameBuffer& target() { return g_targetFb; }

void kickTransition() { g_transitionUntilMs = millis() + TRANSITION_MS; }

void show(uint32_t nowMs, uint32_t dtMs) {
  const Settings& s = settings::get();

  if (g_firstShow) {
    g_firstShow = false;
    g_firstShowMs = nowMs;
  }

  // 1) Smooth the jump to a new target during transitions; otherwise follow exactly
  //    (fast effects like lightning must not be blurred).
  const bool inTransition = (int32_t)(g_transitionUntilMs - nowMs) > 0;
  const float alpha = inTransition ? 1.0f - expf(-(float)dtMs / TRANSITION_TAU_MS) : 1.0f;
  for (uint16_t i = 0; i < g_count; i++) {
    g_current[i].r += (g_target[i].r - g_current[i].r) * alpha;
    g_current[i].g += (g_target[i].g - g_current[i].g) * alpha;
    g_current[i].b += (g_target[i].b - g_current[i].b) * alpha;
  }

  // 2) Soft start after power-up.
  float soft = 1.0f;
  if (s.output.softStartSec > 0) {
    soft = smoothstep((float)(nowMs - g_firstShowMs) / (s.output.softStartSec * 1000.0f));
  }

  // 3) Global limit (perceptual), density mask, gamma → linear PWM, current estimate.
  const float globalLevel = clamp01(s.output.maxBrightness / 100.0f) * soft;
  const uint8_t density = s.light.density == 0 ? 1 : s.light.density;
  const float maPerPixel = s.module.maPerPixel;
  float requestedMa = 0;

  for (uint16_t i = 0; i < g_count; i++) {
    if (i % density != 0) {
      g_pwm[i][0] = g_pwm[i][1] = g_pwm[i][2] = 0;
      continue;
    }
    const float r = powf(clamp01(g_current[i].r * globalLevel), GAMMA);
    const float g = powf(clamp01(g_current[i].g * globalLevel), GAMMA);
    const float b = powf(clamp01(g_current[i].b * globalLevel), GAMMA);
    g_pwm[i][0] = (uint8_t)lroundf(r * 255.0f);
    g_pwm[i][1] = (uint8_t)lroundf(g * 255.0f);
    g_pwm[i][2] = (uint8_t)lroundf(b * 255.0f);
    // Each channel ≈ 1/3 of the full-white pixel current.
    requestedMa += (g_pwm[i][0] + g_pwm[i][1] + g_pwm[i][2]) / 765.0f * maPerPixel;
  }
  requestedMa += QUIESCENT_MA_PER_PIXEL * g_count;

  // 4) Current limiter: scale the whole frame down to fit the LED budget.
  float scale = 1.0f;
  const float budget = s.module.budgetMa;
  const float quiescent = QUIESCENT_MA_PER_PIXEL * g_count;
  if (budget > quiescent && requestedMa > budget) {
    scale = (budget - quiescent) / (requestedMa - quiescent);
  }

  float estimatedMa = quiescent;
  for (uint16_t i = 0; i < g_count; i++) {
    uint8_t r = g_pwm[i][0], g = g_pwm[i][1], b = g_pwm[i][2];
    if (scale < 1.0f) {
      r = (uint8_t)(r * scale);
      g = (uint8_t)(g * scale);
      b = (uint8_t)(b * scale);
      estimatedMa += (r + g + b) / 765.0f * maPerPixel;
    }
    g_strip.setPixelColor(i, r, g, b);  // library maps to the module's colour order
  }
  if (scale >= 1.0f) estimatedMa = requestedMa;

  g_strip.show();

  g_stats.count = g_count;
  g_stats.requestedMa = (uint32_t)requestedMa;
  g_stats.estimatedMa = (uint32_t)estimatedMa;
  g_stats.limiterScale = scale;
  g_stats.softStart = soft;
}

Stats stats() { return g_stats; }

}  // namespace led
}  // namespace tl
