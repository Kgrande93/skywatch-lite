// ---------------------------------------------------------------------
// Skywatch Lite
// Physical LED matrix display for Skywatch (skywatch.grandedata.no) -
// polls /api/matrix and renders live flight data on a 128x64 HUB75
// panel (2x2 grid of 64x32 P2.5 panels).
//
// Board: HiLetgo D1 R32 (ESP32, UNO form factor)
// See README.md for wiring, parts list, and setup instructions.
// ---------------------------------------------------------------------

#include "config.h"
#include "frame.h"
#include "settings_store.h"
#include "wifi_setup.h"
#include "local_web.h"
#include "display.h"
#include "api_client.h"
#include "ota_update.h"
#include <WiFi.h>

static Frame currentFrame;
static unsigned long lastPollAttempt = 0;
static unsigned long pollIntervalMs = 60000;  // updated after each successful fetch, see loop()
static unsigned long lastOtaCheck = 0;

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\nSkywatch Lite " FIRMWARE_VERSION);

  SettingsStore::begin();
  Display::begin();
  Display::setBrightness(SettingsStore::getBrightness());
  Display::renderBootScreen("SKYWATCH LITE", "Starting...");

  WifiSetup::connectOrSetup();  // blocks until connected (or restarts on failure)

  LocalWeb::begin();

  Display::renderBootScreen("CONNECTED", WiFi.localIP().toString());
  delay(1000);

  // First fetch happens immediately in loop() since lastPollAttempt = 0.
}

void loop() {
  LocalWeb::handleClient();

  unsigned long now = millis();

  if (now - lastPollAttempt >= pollIntervalMs || lastPollAttempt == 0) {
    lastPollAttempt = now;
    Frame fetched = Api::fetchFrame(SettingsStore::getServerUrl());
    if (fetched.valid) {
      currentFrame = fetched;
      Display::render(currentFrame);
    }
    // On failure, currentFrame (and the screen) simply stays as it was -
    // a single missed poll shouldn't blank the display. The antenna/
    // server status dots baked into the last good frame will look stale
    // until the next successful poll, which is an acceptable trade-off
    // versus flickering to a blank/error screen on every transient
    // network hiccup.
  }

  if (now - lastOtaCheck >= OTA_CHECK_INTERVAL_MS || lastOtaCheck == 0) {
    lastOtaCheck = now;
    OtaUpdate::checkAndApply();  // no-op and returns quickly if already current
  }

  delay(50);
}
