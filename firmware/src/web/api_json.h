#pragma once
// Settings ⇄ JSON (REST API /api/config). Field names are the public API —
// keep them stable; the panel and backups depend on them.
#include <ArduinoJson.h>

#include "../core/settings.h"

namespace tl {
namespace api {

void settingsToJson(const Settings& s, JsonObject out);

struct ApplyResult {
  bool changed = false;
  bool timeZoneChanged = false;
  bool moduleChanged = false;
  bool restartRequired = false;
  String error;  // empty = OK
};

// Applies only the fields present in `in` (partial update).
ApplyResult applyJson(JsonObjectConst in, Settings& s);

}  // namespace api
}  // namespace tl
