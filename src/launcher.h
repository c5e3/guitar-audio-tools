// App launcher: a carousel showing one app at a time with a big icon.
// Buttons: PWR / B browse, M5 opens the app.
// The last opened app is remembered (NVS) and shown after power-on.
#pragma once
#include "app.h"

namespace launcher {

void begin();                 // call after settings::begin()
void draw();

// Handles a key; returns the app to open, or nullptr.
App *onKey(const Key &key);

// Help page of the launcher (from its menu); returns the number of lines.
int help(const ui::HelpItem *&items);

}  // namespace launcher
