#include "wifi_setup.h"
#include "config.h"
#include "settings_store.h"
#include "display.h"
#include <WiFiManager.h>

// A custom field on WiFiManager's config portal page for the Skywatch
// server URL, so WiFi + server setup happens in one flow instead of two
// separate steps (matches the "combine WiFi and area in one screen"
// decision from planning).
static char serverUrlBuffer[128];

static void saveConfigCallback() {
  SettingsStore::setServerUrl(String(serverUrlBuffer));
  SettingsStore::setSetupDone(true);
}

void WifiSetup::connectOrSetup(bool forcePortal) {
  WiFiManager wm;

  String existingUrl = SettingsStore::getServerUrl();
  existingUrl.toCharArray(serverUrlBuffer, sizeof(serverUrlBuffer));

  WiFiManagerParameter serverUrlParam(
    "server_url", "Skywatch server URL", serverUrlBuffer, sizeof(serverUrlBuffer)
  );
  wm.addParameter(&serverUrlParam);
  wm.setSaveConfigCallback([&serverUrlParam]() {
    strncpy(serverUrlBuffer, serverUrlParam.getValue(), sizeof(serverUrlBuffer));
    saveConfigCallback();
  });

  wm.setAPCallback([](WiFiManager *) {
    Display::renderBootScreen("SETUP MODE", "Connect to Skywatch-Setup");
  });

  bool connected;
  if (forcePortal) {
    Display::renderBootScreen("WIFI RESET", "Starting setup portal");
    connected = wm.startConfigPortal(WIFI_SETUP_AP_NAME);
  } else {
    connected = wm.autoConnect(WIFI_SETUP_AP_NAME);
  }

  if (!connected) {
    // Portal timed out or failed - reboot and try again rather than
    // getting stuck in a half-configured state.
    Display::renderBootScreen("SETUP FAILED", "Restarting...");
    delay(3000);
    ESP.restart();
  }

  strncpy(serverUrlBuffer, serverUrlParam.getValue(), sizeof(serverUrlBuffer));
  if (strlen(serverUrlBuffer) > 0) {
    SettingsStore::setServerUrl(String(serverUrlBuffer));
  }
  SettingsStore::setSetupDone(true);
}
