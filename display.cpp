#include "display.h"
#include "config.h"
#include "font5x7.h"
#include "font3x5.h"
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

static MatrixPanel_I2S_DMA *matrix = nullptr;

// ---- font lookup + drawing ----

static const Glyph5x7* findGlyph5x7(char c) {
  c = toupper(c);
  for (int i = 0; i < FONT_5x7_COUNT; i++) {
    if (FONT_5x7[i].c == c) return &FONT_5x7[i];
  }
  return nullptr;
}

static const Glyph3x5* findGlyph3x5(char c) {
  c = toupper(c);
  for (int i = 0; i < FONT_3x5_COUNT; i++) {
    if (FONT_3x5[i].c == c) return &FONT_3x5[i];
  }
  return nullptr;
}

// Draws text with the big (5x7) font. Returns the x position after the
// last character, so callers can center text by measuring first.
static int drawText5x7(int x, int y, const String &text, uint32_t color) {
  for (unsigned int i = 0; i < text.length(); i++) {
    const Glyph5x7 *g = findGlyph5x7(text[i]);
    if (g) {
      for (int row = 0; row < 7; row++) {
        for (int col = 0; col < 5; col++) {
          if (g->rows[row] & (1 << (4 - col))) {
            matrix->drawPixel(x + col, y + row, color);
          }
        }
      }
    }
    x += 6;  // 5px glyph + 1px gap
  }
  return x;
}

static int drawText3x5(int x, int y, const String &text, uint32_t color) {
  for (unsigned int i = 0; i < text.length(); i++) {
    const Glyph3x5 *g = findGlyph3x5(text[i]);
    if (g) {
      for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 3; col++) {
          if (g->rows[row] & (1 << (2 - col))) {
            matrix->drawPixel(x + col, y + row, color);
          }
        }
      }
    }
    x += 4;  // 3px glyph + 1px gap
  }
  return x;
}

static int measure5x7(const String &text) { return text.length() * 6 - 1; }
static int measure3x5(const String &text) { return text.length() * 4 - 1; }

static void drawText5x7Centered(int y, const String &text, uint32_t color) {
  int w = measure5x7(text);
  int x = (MATRIX_WIDTH - w) / 2;
  drawText5x7(max(0, x), y, text, color);
}

static void drawText3x5Centered(int y, const String &text, uint32_t color) {
  int w = measure3x5(text);
  int x = (MATRIX_WIDTH - w) / 2;
  drawText3x5(max(0, x), y, text, color);
}

// ---- simplified per-airline logos, same shapes as the web mockup ----
// All drawn into an 8x7 box at (x0, y0).

static void drawLogoDefault(int x0, int y0, uint32_t color) {
  const int r = 3;
  const int cx = x0 + 4, cy = y0 + 3;
  for (int y = -r; y <= r; y++) {
    for (int x = -r; x <= r; x++) {
      if (x * x + y * y <= r * r) matrix->drawPixel(cx + x, cy + y, color);
    }
  }
}

// ---- banner (bottom strip, used whenever an ntfy message is active) ----

static void drawBanner(const String &text, uint32_t color) {
  // Bottom 8 rows reserved for the banner. Long text is simply left-
  // aligned and clipped for now - scrolling is a firmware v2 nicety, not
  // needed for the initial hardware bring-up.
  int y = MATRIX_HEIGHT - 7;
  matrix->fillRect(0, MATRIX_HEIGHT - 9, MATRIX_WIDTH, 9, matrix->color565(0, 0, 0));
  drawText3x5(2, y, text.substring(0, 30), color);
}

// ---- status dots (antenna / server, top-right corner) ----

static void drawStatusDots(uint32_t antennaColor, uint32_t serverColor) {
  matrix->drawPixel(MATRIX_WIDTH - 3, 0, antennaColor);
  matrix->drawPixel(MATRIX_WIDTH - 5, 0, antennaColor);
  matrix->drawPixel(MATRIX_WIDTH - 9, 0, serverColor);
  matrix->drawPixel(MATRIX_WIDTH - 11, 0, serverColor);
}

// ---- progress bar (bottom-most rows, unless a banner is showing there) ----

static void drawProgressBar(float progress, uint32_t color) {
  int lit = (int)(progress * MATRIX_WIDTH);
  for (int x = 0; x < MATRIX_WIDTH; x++) {
    uint32_t c = (x < lit) ? color : matrix->color565(20, 24, 28);
    matrix->drawPixel(x, MATRIX_HEIGHT - 2, c);
    matrix->drawPixel(x, MATRIX_HEIGHT - 1, c);
  }
}

void Display::begin() {
  HUB75_I2S_CFG cfg(
    PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN_X * PANEL_CHAIN_Y
  );
  cfg.gpio.r1 = PIN_R1; cfg.gpio.g1 = PIN_G1; cfg.gpio.b1 = PIN_B1;
  cfg.gpio.r2 = PIN_R2; cfg.gpio.g2 = PIN_G2; cfg.gpio.b2 = PIN_B2;
  cfg.gpio.a = PIN_A; cfg.gpio.b = PIN_B; cfg.gpio.c = PIN_C; cfg.gpio.d = PIN_D;
  cfg.gpio.lat = PIN_LAT; cfg.gpio.oe = PIN_OE; cfg.gpio.clk = PIN_CLK;

  matrix = new MatrixPanel_I2S_DMA(cfg);
  matrix->begin();
  matrix->setBrightness8(204);  // ~80% default, overridden by setBrightness() once settings load
  matrix->clearScreen();
}

void Display::setBrightness(uint8_t percent) {
  if (!matrix) return;
  uint8_t val = map(constrain(percent, 0, 100), 0, 100, 0, 255);
  matrix->setBrightness8(val);
}

void Display::renderBootScreen(const String &line1, const String &line2) {
  if (!matrix) return;
  matrix->clearScreen();
  uint32_t white = matrix->color565(245, 247, 250);
  uint32_t grey = matrix->color565(138, 148, 160);
  drawText3x5Centered(20, line1, white);
  drawText3x5Centered(28, line2, grey);
}

void Display::render(const Frame &frame) {
  if (!matrix || !frame.valid) return;

  matrix->clearScreen();
  drawStatusDots(frame.antennaColor, frame.serverColor);

  if (frame.mode == "idle") {
    drawText5x7(10, 1, frame.name, matrix->color565(79, 168, 255));
    drawText3x5(4, 42, frame.line4, matrix->color565(79, 168, 255));
    drawText3x5(4, 53, frame.line5, matrix->color565(79, 168, 255));
  } else {
    if (frame.logo == "default") {
      drawLogoDefault(1, 1, frame.logoColor);
    } else {
      // Named logos (united/hawaiian/southwest) - simplified shapes,
      // same as the web mockup's canvas renderer. Kept minimal here;
      // extend drawLogoDefault-style helpers per name as needed.
      drawLogoDefault(1, 1, frame.accentColor);
    }
    drawText5x7(10, 1, frame.name, matrix->color565(245, 247, 250));
    if (frame.route.length()) drawText3x5(10, 9, frame.route, matrix->color565(199, 206, 212));
    if (frame.actype.length()) drawText3x5(10, 15, frame.actype, matrix->color565(124, 135, 146));
    if (frame.line4.length()) drawText3x5(4, 42, frame.line4, frame.accentColor);
    if (frame.line5.length()) drawText3x5(4, 53, frame.line5, frame.accentColor);
  }

  if (frame.hasBanner) {
    drawBanner(frame.bannerText, matrix->color565(255, 180, 84));
  } else {
    drawProgressBar(frame.progress, matrix->color565(61, 220, 132));
  }

  matrix->flipDMABuffer();
}
