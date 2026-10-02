#include "ui.h"

namespace ui {

M5Canvas canvas;

void begin() {
  // landscape, the M5 button right of the display (rotation 1 would put it left)
  M5.Display.setRotation(3);
  canvas.createSprite(WIDTH, HEIGHT);
}

void clear() { canvas.fillSprite(BLACK); }

void header(const char *title, const char *right) {
  canvas.fillRect(0, 0, WIDTH, HEADER_HEIGHT - 1, NAVY);
  canvas.setTextSize(1);
  canvas.setTextColor(WHITE);
  canvas.setCursor(3, 2);
  canvas.print(title);
  if (right) {
    canvas.setCursor(WIDTH - 3 - canvas.textWidth(right), 2);
    canvas.print(right);
  }
}

void footer(const char *help) {
  canvas.setTextSize(1);
  canvas.setTextColor(YELLOW);
  canvas.setCursor(3, FOOTER_Y);
  canvas.print(help);
}

void drawA4(float a4) {
  if (fabsf(a4 - 440.0f) < 0.01f) return;
  char text[12];
  snprintf(text, sizeof(text), "A4 %.0f", a4);
  canvas.setTextSize(2);
  int w = canvas.textWidth(text);
  canvas.fillRect(WIDTH - w - 6, HEADER_HEIGHT, w + 6, 18, BLACK);
  canvas.setTextColor(ORANGE);
  canvas.setCursor(WIDTH - w - 3, HEADER_HEIGHT + 2);
  canvas.print(text);
  canvas.setTextSize(1);
}

void footerHelp() {
  canvas.setTextSize(1);
  canvas.setTextColor(YELLOW);
  canvas.setCursor(3, FOOTER_Y);
  canvas.print("hold M5: menu");
}

void helpPage(const char *title, const HelpItem *items, int count) {
  canvas.fillSprite(BLACK);
  header(title, "HELP");
  canvas.setFont(&fonts::DejaVu12);
  canvas.setTextSize(1);
  const int lineHeight = 15, keyWidth = 58;
  for (int i = 0; i < count; i++) {
    int y = HEADER_HEIGHT + 3 + i * lineHeight;
    if (!items[i].keys) {                     // explanation, not a key
      canvas.setTextColor(CYAN);
      canvas.setCursor(4, y);
      canvas.print(items[i].text);
      continue;
    }
    canvas.setTextColor(YELLOW);
    canvas.setCursor(4, y);
    canvas.print(items[i].keys);
    canvas.setTextColor(WHITE);
    canvas.setCursor(keyWidth, y);
    canvas.print(items[i].text);
  }
  canvas.setTextColor(WHITE);
  canvas.setCursor(4, HEIGHT - 13);
  canvas.print("any button: close help");
  canvas.setFont(&fonts::Font0);             // back to the default font
}

static int flashValue = 0, flashMax = 30;
static const char *flashLabel = "MIC GAIN";
static const char *flashUnit = "dB";
static uint32_t flashUntil = 0;

void flashLevel(const char *label, int value, int max, const char *unit) {
  flashLabel = label;
  flashValue = value;
  flashMax = max;
  flashUnit = unit;
  flashUntil = millis() + 1500;
}

void flashGain(int db) { flashLevel("MIC GAIN", db, 30, "dB"); }

void drawFlash() {
  if ((int32_t)(flashUntil - millis()) <= 0) return;
  const int w = 180, h = 44, x = (WIDTH - w) / 2, y = (HEIGHT - h) / 2;
  canvas.fillRoundRect(x, y, w, h, 6, NAVY);
  canvas.drawRoundRect(x, y, w, h, 6, YELLOW);
  char text[24];
  snprintf(text, sizeof(text), "%s %d %s", flashLabel, flashValue, flashUnit);
  canvas.setTextSize(2);
  canvas.setTextColor(WHITE);
  canvas.setCursor(x + (w - canvas.textWidth(text)) / 2, y + 6);
  canvas.print(text);
  // bar 0 ... 30 dB
  int barW = w - 20;
  canvas.drawRect(x + 10, y + 28, barW, 9, WHITE);
  canvas.fillRect(x + 11, y + 29, (barW - 2) * flashValue / flashMax, 7, YELLOW);
  canvas.setTextSize(1);
}

void menuPage(const char *title, const char *const *labels, int count, int selected) {
  canvas.fillSprite(BLACK);
  header(title, "MENU");
  canvas.setFont(&fonts::DejaVu12);
  canvas.setTextSize(1);
  const int lineHeight = 15, rows = 7;
  int first = constrain(selected - rows / 2, 0, max(0, count - rows));
  for (int row = 0; row < rows && first + row < count; row++) {
    int i = first + row;
    int y = HEADER_HEIGHT + 2 + row * lineHeight;
    if (i == selected) canvas.fillRect(0, y - 1, WIDTH - 6, lineHeight, NAVY);
    canvas.setTextColor(i == selected ? YELLOW : WHITE);
    canvas.setCursor(6, y + 1);
    canvas.print(labels[i]);
  }
  // scroll bar when the list is longer than the screen
  if (count > rows) {
    int h = HEIGHT - HEADER_HEIGHT - 2;
    canvas.drawFastVLine(WIDTH - 3, HEADER_HEIGHT + 1, h, DARKGREY);
    canvas.fillRect(WIDTH - 4, HEADER_HEIGHT + 1 + h * first / count, 3, h * rows / count, WHITE);
  }
  canvas.setFont(&fonts::Font0);
  footer("PWR/B: move  M5: select  hold: close");
}

void push() { canvas.pushSprite(&M5.Display, 0, 0); }

}  // namespace ui
