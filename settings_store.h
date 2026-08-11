#pragma once
#include <Arduino.h>

// Thin wrapper around ESP32's Preferences (NVS flash) for the handful of
// settings the device itself needs to remember across reboots. Everything
// else (data source, ntfy, area, etc.) lives server-side in Skywatch's own
// settings.json - this device only needs to know *which server* to ask.

namespace SettingsStore {
  void begin();
  String getServerUrl();
  void setServerUrl(const String &url);
  uint8_t getBrightness();
  void setBrightness(uint8_t value);
  bool isSetupDone();
  void setSetupDone(bool done);
  void factoryReset();  // clears everything, forces WiFi + server setup again
}
