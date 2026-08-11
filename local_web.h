#pragma once

namespace LocalWeb {
  // Starts the always-on local settings page (reachable at
  // http://skywatch-lite.local/ once mDNS resolves, or by IP). Lets the
  // user change brightness or trigger a WiFi reconfigure without a
  // factory reset - the "settings should always be reachable" decision
  // from planning.
  void begin();
  void handleClient();  // call from loop()
}
