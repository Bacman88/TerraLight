#pragma once
// ─────────────────────────────────────────────────────────────
//  HTTP server: panel (embedded, gzip) + REST API (ARCHITECTURE.md §8)
//  + captive-portal redirects while the setup AP is active.
// ─────────────────────────────────────────────────────────────
#include <Arduino.h>

namespace tl {
namespace web {

void begin();
void loop();

}  // namespace web
}  // namespace tl
