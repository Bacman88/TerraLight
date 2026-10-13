#pragma once
// Static light: one colour on the whole module (density is applied by the output).
#include "../../core/settings.h"
#include "../frame.h"

namespace tl {
namespace scenes {

inline void renderStatic(const LightSettings& l, FrameBuffer& fb) {
  if (!l.on) {
    fb.clear();
    return;
  }
  const float level = l.brightness / 100.0f;
  RGBf c = rgb8(l.r, l.g, l.b);
  c.r *= level;
  c.g *= level;
  c.b *= level;
  fb.fill(c);
}

}  // namespace scenes
}  // namespace tl
