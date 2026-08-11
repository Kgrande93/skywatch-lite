#include "local_web.h"
#include "config.h"
#include "settings_store.h"
#include "wifi_setup.h"
#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>

static WebServer server(LOCAL_WEB_PORT);

static const char PAGE_TEMPLATE[] PROGMEM = R"HTML(
<!doctype html><html><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Skywatch Lite</title>
<style>
  body{font-family:system-ui,sans-serif;background:#0b0d10;color:#f2f5f7;padding:24px;max-width:360px;margin:0 auto;}
  h1{font-size:18px} label{display:block;font-size:13px;color:#8a94a0;margin-top:16px}
  input{width:100%;padding:8px;margin-top:4px;background:#12161a;border:1px solid #242b31;border-radius:6px;color:#f2f5f7;box-sizing:border-box}
  button{margin-top:18px;padding:9px 16px;background:#3ddc84;border:none;border-radius:6px;color:#0b0d10;font-weight:600}
  .danger{background:#ff4d4d;color:#fff;margin-top:28px}
  .muted{color:#8a94a0;font-size:12px;margin-top:6px}
</style></head><body>
<h1>Skywatch Lite</h1>
<p class="muted">Firmware %FW_VERSION% &middot; %SERVER_URL%</p>
<form method="post" action="/save">
  <label>Brightness (0-100)</label>
  <input type="number" name="brightness" min="0" max="100" value="%BRIGHTNESS%">
  <button type="submit">Save</button>
</form>
<form method="post" action="/reconfigure-wifi" onsubmit="return confirm('This disconnects the device and reopens the setup network. Continue?');">
  <button type="submit" class="danger">Change WiFi</button>
</form>
</body></html>
)HTML";

static void handleRoot() {
  String html = FPSTR(PAGE_TEMPLATE);
  html.replace("%FW_VERSION%", FIRMWARE_VERSION);
  html.replace("%SERVER_URL%", SettingsStore::getServerUrl());
  html.replace("%BRIGHTNESS%", String(SettingsStore::getBrightness()));
  server.send(200, "text/html", html);
}

static void handleSave() {
  if (server.hasArg("brightness")) {
    int val = server.arg("brightness").toInt();
    SettingsStore::setBrightness((uint8_t)constrain(val, 0, 100));
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

static void handleReconfigureWifi() {
  server.send(200, "text/html", "<p>Restarting into setup mode&hellip;</p>");
  delay(500);
  // WiFiManager's own reset + restart is simplest and most reliable way
  // to force a fresh portal on next boot.
  WiFi.disconnect(true, true);
  delay(200);
  ESP.restart();
}

void LocalWeb::begin() {
  if (MDNS.begin(MDNS_HOSTNAME)) {
    MDNS.addService("http", "tcp", LOCAL_WEB_PORT);
  }
  server.on("/", HTTP_GET, handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.on("/reconfigure-wifi", HTTP_POST, handleReconfigureWifi);
  server.begin();
}

void LocalWeb::handleClient() {
  server.handleClient();
}
