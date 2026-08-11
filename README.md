# Skywatch Lite

Physical LED matrix display for [Skywatch](https://github.com/Kgrande93/skywatch)
- polls `/api/matrix` and shows live flight data on a 128x64 HUB75 panel
(2x2 grid of 64x32 P2.5 panels), matching TheFlightWall Mini's screen size
(~14" diagonal).

**Board:** HiLetgo D1 R32 (ESP32, Arduino UNO form factor)

## Status

This is a first pass at the firmware, written and reviewed carefully but
**not yet build-tested or run on real hardware** - there's no ESP32
toolchain or physical panel available in the environment this was written
in. Expect to spend a session or two on normal hardware bring-up
(pin-mapping tweaks, brightness/ghosting tuning) once the parts arrive.
Treat this as a solid, complete starting point, not a guaranteed
drop-in.

## Parts

See the shopping list from planning - in short:
- 4x P2.5 64x32 indoor HUB75 panel (2x2 grid)
- 1x HiLetgo D1 R32 ESP32
- 1x 8-channel TXS0108E logic level shifter (pre-assembled, no soldering)
- Female-female Dupont jumper wires
- 1x 5V 30A (150W) power supply
- 1x power distribution terminal block
- HUB75 IDC flat cables (for daisy-chaining the 4 panels)

## Libraries (install via Arduino IDE Library Manager)

| Library | Author | Notes |
|---|---|---|
| ESP32-HUB75-MatrixPanel-DMA | mrcodetastic (mrfaptastic) | Search "ESP32 HUB75 LED MATRIX PANEL DMA Display" |
| ArduinoJson | bblanchon | v7.x (uses the `JsonDocument` API, not the older `StaticJsonDocument`) |
| WiFiManager | tzapu | Captive portal for WiFi + server URL setup |

Everything else (`WiFi`, `HTTPClient`, `WebServer`, `ESPmDNS`, `Preferences`,
`Update`, `WiFiClientSecure`) ships with the ESP32 Arduino core - no
separate install needed.

**Board setup in Arduino IDE:** install the "esp32" board package (Espressif
Systems) via Boards Manager, then select a generic ESP32 Dev Module board
profile - the D1 R32 doesn't need its own entry, it's a standard ESP32.

## Wiring

Through the TXS0108E level shifter (ESP32 side = 3.3V logic, panel side =
5V logic) - no soldering, push-fit Dupont jumpers throughout:

| Signal | D1 R32 GPIO | Panel HUB75 pin |
|---|---|---|
| R1 | 25 | R1 |
| G1 | 26 | G1 |
| B1 | 27 | B1 |
| R2 | 14 | R2 |
| G2 | 12 | G2 |
| B2 | 13 | B2 |
| A | 23 | A |
| B | 22 | B |
| C | 5 | C |
| D | 17 | D |
| LAT | 4 | LAT |
| OE | 15 | OE |
| CLK | 16 | CLK |
| GND | GND | GND |

E is not used - these are 1/16-scan panels, which don't need it (only
1/32-scan panels do). If `config.h`'s pin numbers don't match your actual
board's silkscreen labels, that's the first thing to check and adjust -
GPIO breakout can vary slightly between D1 R32 board revisions.

**Power:** panels get 5V from the PSU directly (through the distribution
block), *not* through the ESP32. The ESP32 itself is powered via USB
during setup, or 5V/GND from the same distribution block once deployed.
Common ground between the ESP32, level shifter, and panels is required.

## Setup flow

1. First boot with no WiFi configured: creates a `Skywatch-Setup` WiFi
   network. Connect to it with a phone, the captive portal should open
   automatically (or browse to `192.168.4.1`).
2. Pick your home WiFi network and enter the password.
3. Same screen also asks for the Skywatch server URL (defaults to
   `https://skywatch.grandedata.no` - change this if pointing at a
   different instance).
4. Device reboots into normal operation and starts polling `/api/matrix`.

## Changing settings later

No factory reset needed. Visit `http://skywatch-lite.local/` (or the
device's IP) from any browser on the same network:
- Adjust brightness
- "Change WiFi" button - reopens the `Skywatch-Setup` portal on demand

## OTA updates

Checks `github.com/Kgrande93/skywatch-lite` releases once/day. To ship an
update: tag a release, attach a compiled `skywatch-lite.bin` as a release
asset (exact filename matters - see `OTA_ASSET_NAME` in `config.h`), and
every deployed device picks it up within 24 hours. Failed/corrupt
downloads roll back automatically (ESP32's dual-partition OTA) - no
bricking risk from a bad release.

To compile a release binary: Arduino IDE → Sketch → Export Compiled
Binary, then rename the `.ino.bin` output to `skywatch-lite.bin` before
attaching it to the GitHub release.

## File structure

```
skywatch-lite.ino    Main setup()/loop()
config.h              Pins, timing constants, version string
frame.h                Shared struct matching /api/matrix's JSON shape
api_client.*           Fetches + parses /api/matrix
display.*               HUB75 rendering (fonts, logos, layout)
font5x7.h / font3x5.h   Pixel font glyph data (same design as the web mockups)
wifi_setup.*            Captive portal (WiFi + server URL, combined)
local_web.*             Always-on settings page (brightness, WiFi reset)
settings_store.*        NVS-backed persistent config
ota_update.*            GitHub-releases-based OTA
```

## Known gaps for a future pass

- Named airline logos (united/hawaiian/southwest) currently fall back to
  the same plain circle as "default" - the per-airline shapes from the
  web mockup's canvas renderer haven't been ported to the HUB75 drawing
  code yet.
- Banner text longer than ~30 characters is clipped, not scrolled.
- No emergency-squawk-specific visual treatment (flashing/red banner)
  beyond whatever color the server sends in `accent`.
