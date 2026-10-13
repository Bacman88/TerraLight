#include "button.h"

#include "../core/pins.h"

namespace tl {
namespace button {

namespace {

constexpr uint32_t DEBOUNCE_MS = 40;
constexpr uint32_t LONG_PRESS_MS = 5000;

Callback g_onLongPress = nullptr;
bool g_rawLast = false;
uint32_t g_rawChangedMs = 0;
bool g_pressed = false;
uint32_t g_pressedSinceMs = 0;
bool g_fired = false;

}  // namespace

void begin(Callback onLongPress) {
  g_onLongPress = onLongPress;
  pinMode(pins::BUTTON, INPUT_PULLUP);
  // If the button is held at boot we are in a normal boot anyway (download
  // mode never reaches this code) — ignore that press until it is released.
  g_rawLast = digitalRead(pins::BUTTON) == LOW;
  g_pressed = g_rawLast;
  g_fired = g_pressed;
  g_pressedSinceMs = millis();
}

void loop(uint32_t nowMs) {
  const bool raw = digitalRead(pins::BUTTON) == LOW;
  if (raw != g_rawLast) {
    g_rawLast = raw;
    g_rawChangedMs = nowMs;
  }
  if (nowMs - g_rawChangedMs < DEBOUNCE_MS || raw == g_pressed) {
    // stable and unchanged, or still bouncing
  } else {
    g_pressed = raw;
    if (g_pressed) {
      g_pressedSinceMs = nowMs;
      g_fired = false;
    }
  }

  if (g_pressed && !g_fired && nowMs - g_pressedSinceMs >= LONG_PRESS_MS) {
    g_fired = true;
    Serial.println("[button] long press — Wi-Fi reset");
    if (g_onLongPress) g_onLongPress();
  }
}

bool isHeld() { return g_pressed && !g_fired; }

uint32_t heldForMs() { return isHeld() ? millis() - g_pressedSinceMs : 0; }

}  // namespace button
}  // namespace tl
