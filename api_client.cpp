#include "api_client.h"
#include "config.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// Parses a "#rrggbb" string into 16-bit RGB565, the format the HUB75
// library's color565() expects. Falls back to a dim grey on anything
// that doesn't parse, rather than crashing on a malformed color.
static uint32_t parseHexColor(const String &hex) {
  if (hex.length() < 7 || hex[0] != '#') return 0x39C7;  // dim grey fallback
  long val = strtol(hex.c_str() + 1, nullptr, 16);
  uint8_t r = (val >> 16) & 0xFF;
  uint8_t g = (val >> 8) & 0xFF;
  uint8_t b = val & 0xFF;
  // RGB888 -> RGB565
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

Frame Api::fetchFrame(const String &serverUrl) {
  Frame frame;

  if (WiFi.status() != WL_CONNECTED) {
    return frame;  // valid stays false
  }

  HTTPClient http;
  String url = serverUrl + "/api/matrix";
  http.begin(url);
  http.setTimeout(POLL_TIMEOUT_MS);
  int code = http.GET();

  if (code != HTTP_CODE_OK) {
    http.end();
    return frame;
  }

  String body = http.getString();
  http.end();

  // Sized generously for this small, flat JSON payload - matrix.py's
  // response has no nested arrays, so a static/deserializeJson filter
  // isn't needed here.
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, body);
  if (err) {
    return frame;
  }

  frame.mode = doc["mode"] | "idle";
  frame.antennaColor = parseHexColor(doc["antenna"] | "#ff4d4d");
  frame.serverColor = parseHexColor(doc["server"] | "#ff4d4d");

  if (!doc["banner_text"].isNull()) {
    frame.bannerText = doc["banner_text"].as<String>();
    frame.hasBanner = frame.bannerText.length() > 0;
  }

  frame.name = doc["name"] | "";
  frame.route = doc["route"] | "";
  frame.actype = doc["actype"] | "";
  frame.line4 = doc["line4"] | "";
  frame.line5 = doc["line5"] | "";
  frame.accentColor = parseHexColor(doc["accent"] | "#f5f7fa");

  frame.logo = doc["logo"] | "default";
  if (!doc["logo_color"].isNull()) {
    frame.logoColor = parseHexColor(doc["logo_color"].as<String>());
  }

  frame.progress = doc["progress"] | 0.0f;
  frame.valid = true;
  return frame;
}
