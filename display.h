#pragma once
#include "frame.h"

namespace Display {
  void begin();
  void setBrightness(uint8_t percent);  // 0-100
  void render(const Frame &frame);      // draws one full frame to the panels
  void renderBootScreen(const String &line1, const String &line2);  // simple status text during setup/OTA
}
