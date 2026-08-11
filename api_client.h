#pragma once
#include "frame.h"

namespace Api {
  // Fetches and parses the current /api/matrix state from the configured
  // server. Returns a Frame with valid=false on any network/parse error -
  // caller decides what to do (typically: keep showing the last good frame,
  // don't blank the screen on a single transient failure).
  Frame fetchFrame(const String &serverUrl);
}
