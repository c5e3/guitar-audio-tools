// Guitar Audio Tools for the M5Stack M5StickC Plus 1.1
//
// Shows the app launcher, runs the selected app and feeds it with
// microphone samples, button presses and drawing requests. See ARCHITECTURE.md.
//
// Buttons (the same everywhere):
//   M5  (front, big)      click: OK / the main action     hold: menu
//   B   (side)            next / more
//   PWR (other side)      click: previous / less          hold: back to the apps
// In the menu: PWR / B move, M5 selects, holding M5 closes it.
// While a help page is shown, any button closes it.

#include <M5Unified.h>
#include "app.h"
#include "launcher.h"
#include "version.h"
#include "services/audio_in.h"
#include "services/settings.h"
#include "services/ui.h"

App *current = nullptr;         // the running app, nullptr = launcher
bool showingHelp = false;       // help page instead of the app

// ---------- apps ----------

void openApp(App *app) {
  current = app;
  showingHelp = false;
  current->enter();
  if (current->sampleRate() > 0 && !audio_in::start(current->sampleRate(), current->micGain()))
    Serial.println("audio_in::start failed");
}

void closeApp() {
  showingHelp = false;
  audio_in::stop();
  current->exit();
  current = nullptr;
}

// A key goes to the running app, or to the launcher when no app runs.
void deliver(const Key &key) {
  if (current) {
    current->onKey(key);
  } else if (App *app = launcher::onKey(key)) {
    openApp(app);
  }
}

// ---------- menu (hold M5) ----------

// keys of the lines main adds itself
const char KEY_HELP = 1, KEY_BACK = 2, KEY_POWER_OFF = 3;
const int MAX_MENU = 16;

bool menuOpen = false;
int menuSelected = 0;
MenuItem menuItems[MAX_MENU];
const char *menuLabels[MAX_MENU];
int menuCount = 0;

void openMenu() {
  menuCount = 0;
  if (current) {
    const MenuItem *items = nullptr;
    int count = current->menu(items);
    for (int i = 0; i < count && menuCount < MAX_MENU - 2; i++) menuItems[menuCount++] = items[i];
    menuItems[menuCount++] = {"Help", KEY_HELP, false};
    menuItems[menuCount++] = {"Back to the apps", KEY_BACK, false};
  } else {
    menuItems[menuCount++] = {"Help", KEY_HELP, false};
    menuItems[menuCount++] = {"Power off", KEY_POWER_OFF, false};
  }
  for (int i = 0; i < menuCount; i++) menuLabels[i] = menuItems[i].label;
  menuSelected = 0;
  menuOpen = true;
}

void selectMenuItem() {
  const MenuItem &item = menuItems[menuSelected];
  if (!item.repeat) menuOpen = false;
  if (item.key == KEY_HELP) {
    showingHelp = true;
  } else if (item.key == KEY_BACK) {
    if (current) closeApp();
  } else if (item.key == KEY_POWER_OFF) {
    M5.Power.powerOff();
  } else {
    Key key;
    if (item.key == '\r') key.enter = true;
    else key.ch = item.key;
    deliver(key);
  }
}

// ---------- buttons ----------

void handleButtons() {
  bool ok = M5.BtnA.wasClicked();       // released before the hold time
  bool okHold = M5.BtnA.wasHold();
  bool next = M5.BtnB.wasPressed();     // at once: no hold function
  bool prev = M5.BtnPWR.wasClicked();   // the power chip reports it on release
  bool prevHold = M5.BtnPWR.wasHold();
  if (!ok && !okHold && !next && !prev && !prevHold) return;

  if (showingHelp) {                    // any button closes the help page
    showingHelp = false;
    return;
  }

  if (menuOpen) {
    if (next) menuSelected = (menuSelected + 1) % menuCount;
    if (prev) menuSelected = (menuSelected + menuCount - 1) % menuCount;
    if (ok) selectMenuItem();
    if (okHold || prevHold) menuOpen = false;
    return;
  }

  if (okHold) {
    openMenu();
    return;
  }
  if (prevHold) {
    if (current) closeApp();
    return;
  }
  Key key;
  if (ok) key.enter = true;
  else if (next) key.ch = '/';
  else if (prev) key.ch = ',';
  deliver(key);
}

// ---------- main program ----------

void setup() {
  auto cfg = M5.config();
  cfg.internal_mic = true;
  cfg.internal_spk = true;
  M5.begin(cfg);
  Serial.begin(115200);
  Serial.println("Guitar Audio Tools " FIRMWARE_VERSION);
  M5.Display.setBrightness(128);
  ui::begin();
  settings::begin();
  launcher::begin();
}

void loop() {
  M5.update();
  handleButtons();

  if (current) current->tick();

  // hand all waiting microphone samples to the running app
  if (current) {
    static int16_t chunk[256];
    size_t count;
    while ((count = audio_in::read(chunk, 256)) > 0) current->process(chunk, count);
  }

  // redraw about 30 times per second
  static uint32_t lastDraw = 0;
  if (millis() - lastDraw >= 33) {
    lastDraw = millis();
    ui::clear();
    const char *title = current ? current->name() : "Guitar Audio Tools";
    if (showingHelp) {
      const ui::HelpItem *items = nullptr;
      int count = current ? current->help(items) : launcher::help(items);
      ui::helpPage(title, items, count);
    } else if (menuOpen) {
      ui::menuPage(title, menuLabels, menuCount, menuSelected);
      ui::drawFlash();                  // e.g. the new gain after "Mic gain +"
    } else if (current) {
      current->draw(ui::canvas);
      ui::drawFlash();
    } else {
      launcher::draw();
    }
    ui::push();
  }
  delay(1);
}
