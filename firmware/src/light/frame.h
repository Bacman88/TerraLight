#pragma once
#include <Arduino.h>

namespace tl {

// Perceptual colour (gamma-encoded, like an HTML #rrggbb), 0.0–1.0 per
// channel. Scenes render into this; brightness, gamma → linear PWM,
// density and current limiting are applied by led_output.
struct RGBf {
  float r = 0, g = 0, b = 0;
};

struct FrameBuffer {
  RGBf* px = nullptr;
  uint16_t count = 0;

  void fill(const RGBf& c) {
    for (uint16_t i = 0; i < count; i++) px[i] = c;
  }
  void clear() { fill(RGBf{}); }
};

inline RGBf rgb8(uint8_t r, uint8_t g, uint8_t b) { return RGBf{r / 255.0f, g / 255.0f, b / 255.0f}; }

}  // namespace tl
