// ═══════════════════════════════════════════════════════════════
//  TerraLight firmware v2 — ESP32-C3 SuperMini
//  Open source, MIT — https://github.com/Bacman88/TerraLight
//  Docs: docs/FIRMWARE.md (features), docs/ARCHITECTURE.md (design)
//
//  The ESP32-C3 has a single core: nothing in loop() may block.
// ═══════════════════════════════════════════════════════════════
#include <Arduino.h>

#include "core/clock.h"
#include "core/settings.h"
#include "core/version.h"
#include "input/button.h"
#include "input/status_led.h"
#include "light/led_output.h"
#include "light/scenes/scene_static.h"
#include "net/wifi_manager.h"
#include "web/server.h"

using namespace tl;

namespace {

constexpr uint32_t FRAME_INTERVAL_MS = 25;  // 40 FPS
constexpr uint32_t INPUT_INTERVAL_MS = 20;

uint32_t g_lastFrameMs = 0;
uint32_t g_lastInputMs = 0;

void onWifiConnected() { clk::onNetworkUp(); }

void onButtonLongPress() { wifi::forgetAndStartAp(); }

status_led::Pattern statusPattern() {
  if (button::isHeld() && button::heldForMs() > 1000) return status_led::Pattern::On;
  switch (wifi::state()) {
    case wifi::State::AccessPoint: return status_led::Pattern::SlowBlink;
    case wifi::State::Connecting: return status_led::Pattern::FastBlink;
    default: return status_led::Pattern::Off;
  }
}

void renderFrame(uint32_t nowMs, uint32_t dtMs) {
  FrameBuffer& fb = led::target();
  // M1: static light only. M2 adds the day context, scenes and night.
  scenes::renderStatic(settings::get().light, fb);
  led::show(nowMs, dtMs);
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(300);  // give USB-CDC a moment; harmless when no monitor is attached
  Serial.printf("\n%s %s\n", TL_FW_NAME, TL_FW_VERSION);

  status_led::begin();
  settings::begin();
  const Settings& s = settings::get();

  led::begin();
  clk::begin(s.time.tz);
  button::begin(onButtonLongPress);
  wifi::begin(s.system.hostname, onWifiConnected);
  web::begin();

  g_lastFrameMs = millis();
}

void loop() {
  const uint32_t now = millis();

  web::loop();
  wifi::loop(now);

  if (now - g_lastInputMs >= INPUT_INTERVAL_MS) {
    g_lastInputMs = now;
    button::loop(now);
    status_led::update(statusPattern(), now);
  }

  settings::loop(now);
  led::loop(now);

  if (now - g_lastFrameMs >= FRAME_INTERVAL_MS) {
    const uint32_t dt = now - g_lastFrameMs;
    g_lastFrameMs = now;
    renderFrame(now, dt);
  }

  delay(1);  // yield to the Wi-Fi stack
}
