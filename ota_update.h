#pragma once

namespace OtaUpdate {
  // Checks GitHub releases for a newer tag than FIRMWARE_VERSION; if
  // found, downloads the .bin asset and flashes it. Uses ESP32's built-in
  // dual-partition OTA (Update.h), so a failed/corrupt flash rolls back
  // to the currently-running firmware automatically - no bricking risk
  // from a bad download.
  void checkAndApply();
}
