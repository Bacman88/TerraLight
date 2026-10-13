#pragma once
// ─────────────────────────────────────────────────────────────
//  BOOT button (GPIO9) — hold 5 s while running: forget Wi-Fi, start AP.
//  Holding it while powering up enters the ESP32 download mode — that is
//  chip behaviour, not a firmware feature (HARDWARE.md).
// ─────────────────────────────────────────────────────────────
#include <Arduino.h>

namespace tl {
namespace button {

using Callback = void (*)();

void begin(Callback onLongPress);
void loop(uint32_t nowMs);
bool isHeld();            // pressed right now (debounced)
uint32_t heldForMs();     // how long, 0 if not pressed

}  // namespace button
}  // namespace tl
