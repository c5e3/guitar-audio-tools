// Interface that every Guitar Audio Tools app implements.
// main.cpp shows the menu, starts the selected app and feeds it with audio,
// button presses and drawing requests. See ARCHITECTURE.md.
#pragma once
#include <M5Unified.h>
#include "services/ui.h"

// One button press, translated to the key an app understands.
// The M5StickC Plus has three buttons (see main.cpp):
//   M5 (front)  -> enter      = the main action / OK
//   B  (side)   -> ch = '/'   = next / more
//   PWR (side)  -> ch = ','   = previous / less
// Everything else comes from the app's menu (hold M5), which sends the
// characters listed in menu().
struct Key {
  char ch = 0;          // character, 0 if none
  bool enter = false;
};

// One line of an app's menu: the text shown and the key it sends.
// key '\r' = Enter. repeat = the menu stays open after it (e.g. gain + / -,
// which is pressed several times).
struct MenuItem {
  const char *label;
  char key;
  bool repeat;
};

class App {
 public:
  virtual ~App() = default;

  virtual const char *name() const = 0;

  // Microphone set-up used while the app runs; sample rate 0 = the app does not
  // use the microphone (e.g. it plays sound with the buzzer).
  virtual uint32_t sampleRate() const { return 16000; }
  virtual int micGain() const { return 24; }        // dB, 0..30 (digital)

  // Opened / closed: allocate big buffers in enter(), free them in exit().
  virtual void enter() {}
  virtual void exit() {}

  // New audio, called with small chunks of a continuous stream.
  virtual void process(const int16_t *samples, size_t count) = 0;

  // Draw the whole screen into the canvas (about 30 times per second).
  virtual void draw(M5Canvas &canvas) = 0;

  virtual void onKey(const Key &key) {}

  // Called on every pass of the main loop.
  virtual void tick() {}

  // Extra actions shown in the menu (hold M5); main adds "Help" and
  // "Back to apps". Returns the number of items.
  virtual int menu(const MenuItem *&items) const {
    items = nullptr;
    return 0;
  }

  // Help page (from the menu): returns the number of lines.
  virtual int help(const ui::HelpItem *&items) const {
    items = nullptr;
    return 0;
  }
};
