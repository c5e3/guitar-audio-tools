// Chords: a chord dictionary. Choose a root and a chord type and its shape is
// drawn on a horizontal fretboard (high E string at the top, as the guitar is
// seen when playing), with the fingers and the notes of the chord.
//
// Buttons: M5 moves between the three fields ROOT -> TYPE -> SHAPE (the field
// being changed is yellow), PWR / B change it: previous / next root (C C# D Eb
// ... B), chord type (major, m, 7, maj7 ...) or shape of the chord.
// The microphone is not used.
#include <cstring>
#include "apps.h"
#include "theory.h"
#include "tuning.h"
#include "../services/settings.h"
#include "../services/ui.h"

namespace {

const int MAX_SHAPES = 6;

// the fretboard
const int NUT_X = 24;
const int FRET_WIDTH = 42;
const int FRETS_SHOWN = 5;
const int STRING_TOP = 48;         // the high E string
const int STRING_GAP = 13;
const char *STRING_LABELS = "EADGBe";

// the 12 roots, spelled as they are usually written: {letter (0 = C), pitch}
const dsp::SpelledNote ROOTS[12] = {{0, 0}, {0, 1}, {1, 2}, {2, 3}, {2, 4}, {3, 5},
                                    {3, 6}, {4, 7}, {5, 8}, {5, 9}, {6, 10}, {6, 11}};

enum Field { ROOT, TYPE, SHAPE, FIELDS };
const char *FIELD_NAMES[FIELDS] = {"ROOT", "TYPE", "SHAPE"};

class ChordsApp : public App {
 public:
  const char *name() const override { return "Chords"; }
  uint32_t sampleRate() const override { return 0; }   // no microphone

  void enter() override {
    // the last chord: letter * 10000 + pitch * 100 + type
    int saved = settings::getInt("ch_last", 5 * 10000 + 9 * 100 + 4);   // Am7
    root_ = saved / 100 % 100 % 12;                    // ROOTS[i].pitch == i
    type_ = saved % 100 % dsp::CHORD_TYPE_COUNT;
    field_ = ROOT;
    update();
  }

  void exit() override {
    settings::putInt("ch_last", chord_.root.letter * 10000 + chord_.root.pitch * 100 + chord_.type);
  }

  void process(const int16_t *, size_t) override {}

  int help(const ui::HelpItem *&items) const override {
    static const ui::HelpItem HELP[] = {
        {"M5", "next field: root, type,"},
        {"", "shape (yellow = active)"},
        {"PWR/B", "change the active field"},
        {nullptr, "types: m 7 maj7 m7 sus2 sus4"},
        {nullptr, "       dim aug 6 9 add9 5"},
        {nullptr, "orange = root, number = finger"},
        {nullptr, "green bar = barre, x = muted"},
    };
    items = HELP;
    return sizeof(HELP) / sizeof(HELP[0]);
  }

  void onKey(const Key &key) override {
    if (key.enter) field_ = (field_ + 1) % FIELDS;
    int step = key.ch == '/' ? 1 : key.ch == ',' ? -1 : 0;
    if (!step) return;
    if (field_ == ROOT) root_ = (root_ + 12 + step) % 12;
    if (field_ == TYPE) type_ = (type_ + dsp::CHORD_TYPE_COUNT + step) % dsp::CHORD_TYPE_COUNT;
    if (field_ == SHAPE && shapeCount_ > 0) shape_ = (shape_ + shapeCount_ + step) % shapeCount_;
    if (field_ != SHAPE) update();
  }

  void draw(M5Canvas &c) override {
    char right[20];
    if (shapeCount_ > 1) snprintf(right, sizeof(right), "%s  %d/%d", FIELD_NAMES[field_], shape_ + 1, shapeCount_);
    else snprintf(right, sizeof(right), "%s", FIELD_NAMES[field_]);
    ui::header("Chords", right);

    // the name: root and type, the field being changed in yellow
    char rootName[4];
    dsp::noteName(chord_.root, rootName);
    const char *suffix = dsp::CHORD_TYPES[chord_.type].suffix;
    c.setTextSize(3);
    c.setCursor(4, 16);
    c.setTextColor(field_ == ROOT ? YELLOW : WHITE);
    c.print(rootName);
    c.setTextColor(field_ == TYPE ? YELLOW : WHITE);
    c.print(suffix[0] ? suffix : field_ == TYPE ? "maj" : "");
    int x = c.getCursorX() + 10;

    // its notes, next to the name
    c.setTextSize(2);
    dsp::ChordTone tones[6];
    int count = dsp::chordTones(chord_, tones, 6);
    for (int i = 0; i < count; i++) {
      char note[6];
      dsp::noteName(tones[i].note, note);
      c.setTextColor(i == 0 ? ORANGE : GREEN);
      c.setCursor(x, 20);
      c.print(note);
      x += c.textWidth(note) + 6;   // C#add9: C# E# G# D# just fits
    }

    if (shapeCount_ > 0) drawShape(c, shapes_[shape_]);
    ui::footerHelp();
  }

 private:
  void update() {
    dsp::Chord chord = {ROOTS[root_], type_};
    bool changed = chord.root.pitch != chord_.root.pitch || chord.type != chord_.type;
    chord_ = chord;
    shapeCount_ = dsp::chordVoicings(chord_, shapes_, MAX_SHAPES);
    if (changed || shape_ >= shapeCount_) shape_ = 0;
  }

  static int stringY(int s) { return STRING_TOP + (5 - s) * STRING_GAP; }

  bool isRoot(int s, int fret) const {
    return (dsp::GUITAR_MIDI[s] + fret) % 12 == chord_.root.pitch;
  }

  void drawShape(M5Canvas &c, const dsp::Voicing &v) {
    // which frets to show: from the nut, or from the lowest fret used
    int lowest = 99, highest = 0;
    for (int s = 0; s < 6; s++) {
      if (v.fret[s] > 0) {
        lowest = std::min(lowest, (int)v.fret[s]);
        highest = std::max(highest, (int)v.fret[s]);
      }
    }
    int first = highest <= FRETS_SHOWN ? 1 : lowest;
    int right = NUT_X + FRETS_SHOWN * FRET_WIDTH;

    // strings, frets, the nut, fret numbers
    c.setTextSize(1);
    for (int s = 0; s < 6; s++) {
      int y = stringY(s);
      c.drawFastHLine(NUT_X, y, right - NUT_X, LIGHTGREY);
      c.setTextColor(YELLOW);
      c.setCursor(1, y - 3);
      c.print(STRING_LABELS[s]);
    }
    for (int f = 0; f <= FRETS_SHOWN; f++)
      c.drawFastVLine(NUT_X + f * FRET_WIDTH, stringY(5), stringY(0) - stringY(5) + 1, LIGHTGREY);
    if (first == 1) c.fillRect(NUT_X - 2, stringY(5), 3, stringY(0) - stringY(5) + 1, WHITE);
    c.setTextColor(WHITE);
    for (int f = 0; f < FRETS_SHOWN; f++) {
      char number[4];
      snprintf(number, sizeof(number), "%d", first + f);
      c.setCursor(NUT_X + f * FRET_WIDTH + FRET_WIDTH / 2 - c.textWidth(number) / 2, stringY(0) + 6);
      c.print(number);
    }

    // barre: the index finger over several strings at one fret
    int barreFret = 0, barreLow = 6, barreHigh = -1;
    for (int s = 0; s < 6; s++) {
      if (v.finger[s] == 1 && v.fret[s] > 0) {
        barreFret = v.fret[s];
        barreLow = std::min(barreLow, s);
        barreHigh = std::max(barreHigh, s);
      }
    }
    if (barreHigh > barreLow) {
      int x = fretX(barreFret, first);
      c.fillRoundRect(x - 5, stringY(barreHigh) - 5, 11, stringY(barreLow) - stringY(barreHigh) + 11, 5, GREEN);
    }

    // muted / open strings and the fingers
    for (int s = 0; s < 6; s++) {
      int y = stringY(s);
      int fret = v.fret[s];
      if (fret == dsp::MUTED) {
        c.setTextColor(RED);
        c.setCursor(12, y - 3);
        c.print("x");
      } else if (fret == 0) {
        c.drawCircle(15, y, 4, isRoot(s, 0) ? ORANGE : GREEN);
      } else {
        int x = fretX(fret, first);
        c.fillCircle(x, y, 6, isRoot(s, fret) ? ORANGE : GREEN);
        c.setTextColor(BLACK);
        c.setCursor(x - 2, y - 3);
        c.print((int)v.finger[s]);
      }
    }
  }

  static int fretX(int fret, int first) { return NUT_X + (fret - first) * FRET_WIDTH + FRET_WIDTH / 2; }

  int root_ = 9, type_ = 0, field_ = ROOT;
  dsp::Chord chord_ = {{0, 0}, -1};     // -1: update() always sees a change first
  dsp::Voicing shapes_[MAX_SHAPES];
  int shapeCount_ = 0, shape_ = 0;
};

ChordsApp instance;

}  // namespace

App *chordsApp() { return &instance; }
