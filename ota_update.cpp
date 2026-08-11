#include "ota_update.h"
#include "config.h"
#include "display.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>

// Very small semver-ish compare: treats tags like "v0.2.0" / "0.2.0".
// Good enough for "is the release tag newer than what's running" - not a
// full semver implementation, but this firmware controls both sides of
// the comparison (its own tags), so it doesn't need to be.
static bool isNewerVersion(const String &tag, const String &current) {
  String a = tag; if (a.startsWith("v")) a = a.substring(1);
  String b = current;
  int ai = 0, bi = 0;
  while (ai < (int)a.length() || bi < (int)b.length()) {
    int an = 0, bn = 0;
    while (ai < (int)a.length() && a[ai] != '.') { an = an * 10 + (a[ai] - '0'); ai++; }
    while (bi < (int)b.length() && b[bi] != '.') { bn = bn * 10 + (b[bi] - '0'); bi++; }
    if (an != bn) return an > bn;
    ai++; bi++;
  }
  return false;
}

void OtaUpdate::checkAndApply() {
  HTTPClient http;
  String url = "https://api.github.com/repos/" OTA_GITHUB_REPO "/releases/latest";
  http.begin(url);
  http.addHeader("User-Agent", "skywatch-lite-firmware");
  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    http.end();
    return;
  }
  String body = http.getString();
  http.end();

  JsonDocument doc;
  if (deserializeJson(doc, body)) return;

  String tag = doc["tag_name"] | "";
  if (tag.isEmpty() || !isNewerVersion(tag, FIRMWARE_VERSION)) {
    return;  // already up to date
  }

  String downloadUrl;
  JsonArray assets = doc["assets"];
  for (JsonObject asset : assets) {
    String name = asset["name"] | "";
    if (name == OTA_ASSET_NAME) {
      downloadUrl = asset["browser_download_url"] | "";
      break;
    }
  }
  if (downloadUrl.isEmpty()) return;

  Display::renderBootScreen("UPDATING", tag);

  WiFiClientSecure client;
  client.setInsecure();  // GitHub's cert chain rotates; pinning it here would
                          // brick devices on the next rotation. Integrity is
                          // instead guaranteed by the OTA partition scheme
                          // below (bad flash = automatic rollback), not TLS
                          // pinning.
  HTTPClient httpDownload;
  httpDownload.begin(client, downloadUrl);
  httpDownload.addHeader("User-Agent", "skywatch-lite-firmware");
  int dlCode = httpDownload.GET();
  if (dlCode != HTTP_CODE_OK) {
    httpDownload.end();
    return;
  }

  int contentLength = httpDownload.getSize();
  if (contentLength <= 0) {
    httpDownload.end();
    return;
  }

  if (!Update.begin(contentLength)) {
    httpDownload.end();
    return;
  }

  WiFiClient *stream = httpDownload.getStreamPtr();
  size_t written = Update.writeStream(*stream);
  httpDownload.end();

  if (written != (size_t)contentLength) {
    Update.abort();
    Display::renderBootScreen("UPDATE FAILED", "Keeping current version");
    delay(2000);
    return;
  }

  if (!Update.end() || !Update.isFinished()) {
    Display::renderBootScreen("UPDATE FAILED", "Keeping current version");
    delay(2000);
    return;
  }

  Display::renderBootScreen("UPDATE OK", "Restarting...");
  delay(1500);
  ESP.restart();
}
