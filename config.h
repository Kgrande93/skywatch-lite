#pragma once

// ---------------------------------------------------------------------
// Skywatch Lite - hardware & build configuration
// ---------------------------------------------------------------------

#define FIRMWARE_VERSION "0.1.0"

// GitHub repo this firmware checks for updates against (owner/repo).
// Update instructions in README.md explain how releases must be tagged
// and what asset name the OTA updater looks for.
#define OTA_GITHUB_REPO "Kgrande93/skywatch-lite"
#define OTA_ASSET_NAME  "skywatch-lite.bin"
#define OTA_CHECK_INTERVAL_MS (24UL * 60UL * 60UL * 1000UL)  // once/day

// ---- Panel geometry: 2x2 grid of 64x32 P2.5 HUB75 panels = 128x64 total ----
#define PANEL_RES_X 64
#define PANEL_RES_Y 32
#define PANEL_CHAIN_X 2   // panels across
#define PANEL_CHAIN_Y 2   // panels down
#define MATRIX_WIDTH  (PANEL_RES_X * PANEL_CHAIN_X)   // 128
#define MATRIX_HEIGHT (PANEL_RES_Y * PANEL_CHAIN_Y)   // 64

// ---- HUB75 pin mapping for HiLetgo D1 R32 (UNO form factor) ----
// These are the GPIOs actually broken out on the D1 R32's UNO-shaped
// header. Wired through the TXS0108E level shifter (3.3V ESP32 side ->
// 5V panel side) - see README.md "Wiring" for the full pin-by-pin table.
// If you're using a different ESP32 board, these almost certainly need
// to change to match whatever GPIOs are actually broken out on it.
#define PIN_R1  25
#define PIN_G1  26
#define PIN_B1  27
#define PIN_R2  14
#define PIN_G2  12
#define PIN_B2  13
#define PIN_A   23
#define PIN_B   22
#define PIN_C   5
#define PIN_D   17
#define PIN_E   -1   // 1/32 scan panels need an E line - unused here (1/16 scan panels don't need it)
#define PIN_LAT  4
#define PIN_OE  15
#define PIN_CLK 16

// ---- Persistent storage keys (NVS via Preferences) ----
#define PREFS_NAMESPACE "skywatch"
#define PREF_SERVER_URL "server_url"
#define PREF_BRIGHTNESS "brightness"
#define PREF_SETUP_DONE "setup_done"

#define DEFAULT_SERVER_URL "https://skywatch.grandedata.no"
#define DEFAULT_BRIGHTNESS 80

// ---- Networking ----
#define WIFI_SETUP_AP_NAME "Skywatch-Setup"
#define POLL_TIMEOUT_MS 8000
#define LOCAL_WEB_PORT 80
#define MDNS_HOSTNAME "skywatch-lite"
