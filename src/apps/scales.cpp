// Scales: a scale on the whole fretboard (frets 0-12 or 12-24), high E string
// at the top as in the Chords app. The root is orange, the other notes green,
// the "blue note" (b5) of the blues scale light blue. Small dots without note
// names: 13 columns of labelled dots were too busy on the small display.
//
// Buttons: M5 moves between the fields KEY -> SCALE -> FRETS (the field being
// changed is yellow), PWR / B change it: the key (C C# D Eb ... B), the scale,
// frets 0-12 / 12-24. No microphone.
#include <cstring>
#include "apps.h"
#include "theory.h"
#include "tuning.h"
#include "../services/settings.h"
#include "../services/ui.h"

namespace {

const int NUT_X = 24;              // line after the first column (open strings / fret 12)
const int FRET_WIDTH = 18;
const int FRETS_SHOWN = 12;        // after the first column
const int STRING_TOP = 42;         // the high E string
const int STRING_GAP = 14;
const int DOT_RADIUS = 4;
const char *STRING_LABELS = "EADGBe";
const int MARKED_FRETS[] = {3, 5, 7, 9, 12, 15, 17, 19, 21, 24};
// the 12 keys, spelled as they are usually written: {letter (0 = C), pitch}
const dsp::SpelledNote KEYS[12] = {{0, 0}, {0, 1}, {1, 2}, {2, 3}, {2, 4}, {3, 5},
                                   {3, 6}, {4, 7}, {5, 8}, {5, 9}, {6, 10}, {6, 11}};

enum Field { KEY, SCALE, FRETS, FIELDS };
const char *FIELD_NAMES[FIELDS] = {"KEY", "SCALE", "FRETS"};

class ScalesApp : public App {
 public:
  const char *name() const override { return "Scales"; }
  uint32_t sampleRate() const override { return 0; }   // no microphone

  void enter() override {
    int root = settings::getInt("sc_root", 5 * 100 + 9);   // letter * 100 + pitch: A
    root_ = KEYS[root % 100 % 12];                       // KEYS[i].pitch == i
    type_ = constrain(settings::getInt("sc_type", 0), 0, dsp::SCALE_TYPE_COUNT - 1);
    firstFret_ = 0;
    field_ = KEY;
    update();
  }

  void exit() override {
    settings::putInt("sc_root", root_.letter * 100 + root_.pitch);
    settings::putInt("sc_type", type_);
  }

  void process(const int16_t *, size_t) override {}

  int help(const ui::HelpItem *&items) const override {
    static const ui::HelpItem HELP[] = {
        {"M5", "next field: key, scale,"},
        {"", "frets (yellow = active)"},
        {"PWR/B", "change the active field"},
        {nullptr, "orange = root (1)"},
        {nullptr, "blue = blue note (blues)"},
    };
    items = HELP;
    return sizeof(HELP) / sizeof(HELP[0]);
  }

  void onKey(const Key &key) override {
    if (key.enter) field_ = (field_ + 1) % FIELDS;
    int step = key.ch == '/' ? 1 : key.ch == ',' ? -1 : 0;
    if (!step) return;
    if (field_ == KEY) root_ = KEYS[(root_.pitch + 12 + step) % 12];
    if (field_ == SCALE) type_ = (type_ + dsp::SCALE_TYPE_COUNT + step) % dsp::SCALE_TYPE_COUNT;
    if (field_ == FRETS) firstFret_ = firstFret_ ? 0 : 12;
    update();
  }

  void draw(M5Canvas &c) override {
    char right[24];
    snprintf(right, sizeof(right), "%s  %s", FIELD_NAMES[field_], firstFret_ ? "frets 12-24" : "frets 0-12");
    ui::header("Scales", right);

    // "A minor pentatonic", the field being changed in yellow
    char root[6];
    dsp::noteName(root_, root);
    c.setTextSize(2);
    c.setTextColor(field_ == KEY ? YELLOW : ORANGE);
    c.setCursor(4, 16);
    c.print(root);
    c.setTextColor(field_ == SCALE ? YELLOW : WHITE);
    c.print(" ");
    c.print(dsp::SCALE_TYPES[type_].name);

    drawFretboard(c);
    ui::footerHelp();
  }

 private:
  void update() {
    noteCount_ = dsp::scaleNotes(root_, type_, notes_, 7);
  }

  static int stringY(int s) { return STRING_TOP + (5 - s) * STRING_GAP; }

  // x of the centre of a fret column; column 0 = left of the nut line
  static int columnX(int column) {
    return column == 0 ? NUT_X - 8 : NUT_X + (column - 1) * FRET_WIDTH + FRET_WIDTH / 2;
  }

  void drawFretboard(M5Canvas &c) {
    int right = NUT_X + FRETS_SHOWN * FRET_WIDTH - 1;
    c.setTextSize(1);
    for (int s = 0; s < 6; s++) {
      c.drawFastHLine(NUT_X - 14, stringY(s), right - NUT_X + 15, LIGHTGREY);
      c.setTextColor(YELLOW);
      c.setCursor(1, stringY(s) - 3);
      c.print(STRING_LABELS[s]);
    }
    int top = stringY(5), height = stringY(0) - stringY(5) + 1;
    for (int f = 1; f <= FRETS_SHOWN; f++) c.drawFastVLine(NUT_X + f * FRET_WIDTH - 1, top, height, LIGHTGREY);
    if (firstFret_ == 0) c.fillRect(NUT_X - 2, top, 3, height, WHITE);   // the nut
    else c.drawFastVLine(NUT_X, top, height, LIGHTGREY);

    // numbers of the marked frets (3, 5, 7, 9, 12...) under the strings
    c.setTextColor(field_ == FRETS ? YELLOW : WHITE);
    for (int fret : MARKED_FRETS) {
      int column = fret - firstFret_;
      if (column < 0 || column > FRETS_SHOWN) continue;
      char number[4];
      snprintf(number, sizeof(number), "%d", fret);
      c.setCursor(columnX(column) - c.textWidth(number) / 2, stringY(0) + 9);
      c.print(number);
    }

    // the notes of the scale
    for (int s = 0; s < 6; s++) {
      for (int column = 0; column <= FRETS_SHOWN; column++) {
        int pitch = (dsp::GUITAR_MIDI[s] + firstFret_ + column) % 12;
        for (int i = 0; i < noteCount_; i++) {
          if (notes_[i].note.pitch != pitch) continue;
          drawNote(c, columnX(column), stringY(s), notes_[i], i == 0);
          break;
        }
      }
    }
  }

  void drawNote(M5Canvas &c, int x, int y, const dsp::ChordTone &note, bool root) {
    bool blueNote = strncmp(note.degree, "b5", note.degreeLength) == 0 && note.degreeLength == 2;
    c.fillCircle(x, y, DOT_RADIUS, root ? ORANGE : blueNote ? CYAN : GREEN);
  }

  dsp::SpelledNote root_ = {5, 9};
  int type_ = 0, firstFret_ = 0, field_ = KEY;
  dsp::ChordTone notes_[7];
  int noteCount_ = 0;
};

ScalesApp instance;

}  // namespace

App *scalesApp() { return &instance; }
