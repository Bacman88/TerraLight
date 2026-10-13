#pragma once
// ─────────────────────────────────────────────────────────────
//  LED output — the last stage of the pipeline (ARCHITECTURE.md §5):
//    scene frame → smoothing → density mask → brightness limits
//    → soft start → gamma → current limiter → colour order → show()
//
//  Scenes know nothing about the power supply, colour order or density.
// ─────────────────────────────────────────────────────────────
#include <Arduino.h>

#include "frame.h"

namespace tl {
namespace led {

struct Stats {
  uint16_t count;
  uint32_t estimatedMa;  // estimated LED current of the last frame (after limiting)
  uint32_t requestedMa;  // what the frame would draw without the limiter
  float limiterScale;    // 1.0 = no limiting
  float softStart;       // 0.0–1.0 fade-in after power-up
};

void begin();
void loop(uint32_t nowMs);  // reconfigures on settings changes

// Scene target for the next frame.
FrameBuffer& target();

// Pushes the frame to the LEDs. dtMs = time since the previous frame.
void show(uint32_t nowMs, uint32_t dtMs);

// Short smooth transition after a settings change (e.g. new colour).
void kickTransition();

Stats stats();

}  // namespace led
}  // namespace tl
