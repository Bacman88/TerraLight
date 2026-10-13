#pragma once
// Onboard blue LED of the SuperMini (GPIO8, active LOW) as a Wi-Fi status
// indicator — so the main light never has to blink for diagnostics.
//   AP mode        → slow blink (1 s)
//   connecting     → fast blink (0.2 s)
//   button held    → steady on
//   connected/idle → off
#include <Arduino.h>

#include "../core/pins.h"

namespace tl {
namespace status_led {

enum class Pattern : uint8_t { Off, SlowBlink, FastBlink, On };

inline void begin() {
  pinMode(pins::STATUS_LED, OUTPUT);
  digitalWrite(pins::STATUS_LED, HIGH);  // off
}

inline void update(Pattern p, uint32_t nowMs) {
  bool on = false;
  switch (p) {
    case Pattern::SlowBlink: on = (nowMs / 1000) % 2; break;
    case Pattern::FastBlink: on = (nowMs / 200) % 2; break;
    case Pattern::On: on = true; break;
    default: break;
  }
  digitalWrite(pins::STATUS_LED, on ? LOW : HIGH);
}

}  // namespace status_led
}  // namespace tl
