#include "settings_store.h"
#include "config.h"
#include <Preferences.h>

static Preferences prefs;

void SettingsStore::begin() {
  prefs.begin(PREFS_NAMESPACE, false);
}

String SettingsStore::getServerUrl() {
  return prefs.getString(PREF_SERVER_URL, DEFAULT_SERVER_URL);
}

void SettingsStore::setServerUrl(const String &url) {
  prefs.putString(PREF_SERVER_URL, url);
}

uint8_t SettingsStore::getBrightness() {
  return prefs.getUChar(PREF_BRIGHTNESS, DEFAULT_BRIGHTNESS);
}

void SettingsStore::setBrightness(uint8_t value) {
  prefs.putUChar(PREF_BRIGHTNESS, value);
}

bool SettingsStore::isSetupDone() {
  return prefs.getBool(PREF_SETUP_DONE, false);
}

void SettingsStore::setSetupDone(bool done) {
  prefs.putBool(PREF_SETUP_DONE, done);
}

void SettingsStore::factoryReset() {
  prefs.clear();
}
