#pragma once
#include <Arduino.h>

// Mirrors the JSON shape Skywatch's /api/matrix endpoint returns
// (see matrix.py::build_matrix_response on the server). This is the one
// contract between backend and device - if the server's schema changes,
// this struct (and Api::fetchFrame's parsing) must change to match.
struct Frame {
  bool valid = false;          // false if the last fetch failed - caller should keep showing the previous frame

  String mode;                 // "flight" | "idle"
  uint32_t antennaColor = 0;   // packed RGB565, from the hex color string
  uint32_t serverColor = 0;

  String bannerText;           // empty if no ntfy banner is active
  bool hasBanner = false;

  String name;                 // flight number / "SKYWATCH" for idle
  String route;
  String actype;
  String line4;
  String line5;
  uint32_t accentColor = 0;

  String logo;                 // "united" | "hawaiian" | "southwest" | "default"
  uint32_t logoColor = 0;      // only used when logo == "default"

  float progress = 0.0f;       // 0.0-1.0
};
