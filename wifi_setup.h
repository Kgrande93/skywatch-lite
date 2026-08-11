#pragma once
#include <Arduino.h>

namespace WifiSetup {
  // Blocking call: brings up the WiFi-Setup captive portal (SSID from
  // config.h) if no WiFi credentials are stored yet, or if forcePortal is
  // true (used by the "change WiFi" button in the local web UI). Returns
  // once connected to a real network. Also collects the Skywatch server
  // URL as part of the same flow and saves it via SettingsStore.
  void connectOrSetup(bool forcePortal = false);
}
