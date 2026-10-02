# Guitar Audio Tools for M5StickC Plus 1.1

Audio tools for guitarists and musicians on the M5Stack **M5StickC Plus 1.1**
(ESP32-PICO-D4, AXP192), using its built-in SPM1423 PDM microphone: a precise
tuner, a strum tuner for all six strings at once, BPM detector, spectrum
analyser, sound level meter and more.

Ported from the Cardputer ADV version. The display (240 x 135) is the same,
the keyboard is replaced by the three buttons (see *Controls*). Not for the
M5StickC Plus2 (different power button and power chip). Build and upload
it yourself (below). The screenshots are from the Cardputer; the screens are
the same except for the bottom line.

| | |
|---|---|
| ![Launcher](docs/images/launcher.png) | ![Guitar tuner](docs/images/tuner.png) |
| Launcher: PWR / B browse, M5 open | Guitar tuner: E4, 3 cents flat |
| ![Strum tuner](docs/images/strum_tuner.png) | ![Metronome](docs/images/metronome.png) |
| Strum tuner: all six strings from one strum | Metronome: 4/4, the frame flashes on the beat |
| ![Chords](docs/images/chords.png) | ![Scales](docs/images/scales.png) |
| Chords: choose root and type, see the shape | Scales: A minor pentatonic on the neck |
| ![Spectrum](docs/images/spectrum.png) | ![Decibel meter](docs/images/decibel_meter.png) |
| Spectrum: 26 third-octave bands | Decibel meter: dBA, Leq, Max, Min |

## Apps
Version **1.0.0-beta.2** (shown on the launcher's help page and printed on
the serial console at start). In the order of the launcher:

1. **Guitar tuner** – needle gauge ±50 cents, note name
2. **Strum tuner** – strum all six open strings, see which are out of tune
3. **Metronome** – clicks with accents, tempo from BPM or tapping
4. **BPM detector** – from music or by tapping (tap tempo)
5. **Chords** – chord dictionary: type a name, see the shape on the fretboard
6. **Scales** – scales on the whole fretboard
7. **Intonation** – guitar setup: open string (or 12th-fret harmonic) vs. the
   12th fret, tells which way to move the saddle, overview of all 6 strings
8. **Spectrum analyser** – green/yellow/red bars (16 bars or third octaves)
9. **Decibel meter** – calibrated dB SPL, max, average (Leq), A-weighting
10. **Mic test** – diagnostics

The firmware is meant for musicians and guitarists. (A night/snoring monitor
was dropped for that reason. Checking guitar intonation up the neck needs no
extra app: the tuner in CHROMATIC mode shows any note and its deviation.
A vocal trainer was dropped too: the chord and scale reference fits better.)

Design and development order: [ARCHITECTURE.md](ARCHITECTURE.md).
Current state: all apps done and tested on the device (beta).

## Controls
Three buttons, the same scheme everywhere (hold the StickC with the M5 button
to the right of the display):

| Button | Click | Hold |
|--------|-------|------|
| **M5** (front, big) | OK / the app's main action | open the **menu** |
| **B** (side) | next / more (+) | – |
| **PWR** (other side) | previous / less (−) | back to the app list |

The menu (hold M5) lists the app's other functions (microphone gain, modes,
calibration...), then **Help** and **Back to the apps**. In the menu PWR / B move,
M5 selects, holding M5 closes it. Entries you usually press several times
(gain + / −, tempo ± 5...) keep the menu open, and the new value is flashed
on screen. Microphone gain is shown for 1.5 s as "MIC GAIN 27 dB" and
remembered per app. The help page closes with any button. In the launcher
the menu has Help and Power off. (Holding PWR for 6 s switches the StickC off
in hardware.)

In the text below, key names from the Cardputer version map as follows:
`Enter` = M5, `,` / `/` = PWR / B, everything else = an entry in the menu.

- Launcher: PWR / B browse the apps (round in both directions), M5 opens
  one. After power-on the last opened app is selected (saved by its title).
- Decibel meter: `a` A/Z weighting, `Enter` (or `r`) reset Leq/Max/Min,
  `c` calibration (`;`/`.` ±0.5 dB, `,`/`/` ±5 dB, `Enter` saves, `c` cancels).
  Samples at 32 kHz; A-weighting follows IEC 61672 within 0.25 dB up to 8 kHz.
  Automatic range: (digital) gain 18 dB, switches to 0 dB for very loud sound.
  Time weighting Fast (1/8 s); the big number is rewritten twice per second
  like on real meters, the bar moves smoothly.
- Guitar tuner: `Enter` (or `m`) GUITAR / CHROMATIC mode, `,`/`/` reference pitch A4 −1/+1 Hz
  (430–450; when it is not 440, "A4 438" is shown big and orange at the top right in
  the tuner, Strum tuner and Intonation), `;`/`.` microphone gain. Needle ±50 cents, green centre ±3 cents.
  GUITAR mode always shows the nearest string of the standard tuning, even when
  it is more than a semitone off ("E −90, tune up"). Measured accuracy with
  tones from a laptop speaker (220/440/880 Hz): within ±0.15 cents.
  The needle moves only after 3 agreeing, clearly periodic readings (pluck noise
  is not shown); GUITAR mode searches 60–420 Hz only (no octave errors at the
  pluck); a sub-harmonic of the ringing note (low E resonating while the high E
  decays) is ignored; a new pluck (+6 dB) starts over. For 0.5 s after a
  pluck the needle is thin and light: the string starts sharp and settles
  (the low E by 20–35 cents in the first second). Below 100 Hz the needle is
  smoothed twice as strongly.
- Strum tuner: strum all six open strings; after about 1.2 s one column per
  string shows the deviation (marker up = sharp, down = flat, green centre
  ±3 cents; an arrow above the column says which way to tune: yellow for
  10–50 cents, red for more than 50; "?" = string not heard). Weak strums,
  handling noise and steady background tones in the room are ignored.
  `;`/`.` microphone gain, `Enter` clears the result, `c` calibration.
  Accuracy on synthetic chords ±1.3 cents. In a real strum the low strings read
  flat compared with the single-string tuner (lighter pluck = less pitch glide,
  inharmonic wound strings): a default correction measured on an acoustic
  guitar is applied (E −5.3, A −6.9, D −1.9, G −0.8, B +4.4, E −0.2 cents;
  an unplugged electric guitar read much flatter on the low E, −23.6). For another guitar or new strings: tune with the tuner,
  press `c`, strum 3 times (saved; `Enter` in that screen restores the default).
- Spectrum: 16 bars 60 Hz – 16 kHz of green/yellow/red blocks, FFT of 2048
  samples at 32 kHz every 32 ms, bars fall slowly, peaks hold 0.8 s. Automatic
  sensitivity: the scale jumps to the loudest band and recovers 6 dB/s.
  `Enter` (or `p`) peaks on/off, `,` / `/` (or `r`) bar range 20 / 30 / 40 dB.
  `m` switches to 26 ISO third-octave bands 50 Hz – 16 kHz (FFT of 4096 samples)
  for finding feedback or room resonances; the mode is remembered. Limits of
  this mode: the lowest bands are only 11-15 Hz wide (2 FFT bins), so a low tone
  also lights the neighbouring bar about 8-11 dB lower (the highest bar is the
  right one); the 16k band is cut at 16 kHz (half the sample rate) and reads
  about 3 dB low. If the memory is short, the 16 bars stay (a warning is shown).
  A bank of filters (like professional analysers) would fix the first two.
- BPM: AUTO listens to music (spectral-flux onsets in ~20 bands, a pulse comb
  with the half/double tempo "family" chooses the beat, autocorrelation over
  2–4 beats refines it; first reading after ~3 s, then twice per second over up
  to 6 s). The shown tempo keeps its octave; a new tempo must last 1.5 s,
  2/3 or 3/2 of the shown one 3 s. Grey + "uncertain" when the rhythm is not
  clear. `Enter` or space switches to tapping at once (tempo after 3 taps,
  exactly as tapped); 3 s without a tap: listening again. A dot flashes on the
  beat. `,` /2, `/` x2, `r` restart, `d` dump onset data (serial).
  Tested on songs played from a phone: Sandstorm 136.1, Levels 126.0, Bad
  Romance 119, Thunderstruck 133, Another One Bites the Dust 110.
- Metronome: M5 start/stop, PWR / B tempo ±1 (30–250 BPM); menu: ±5, tap
  tempo (select it, then press M5 on the beat), 2/4 3/4 4/4 6/8 (accent on 1;
  in 6/8 also on 4), volume, the last tempo from the BPM app. The clicks are
  short tones on the StickC's buzzer (higher on the first beat). The clicks are
  timed by a task of their own on the other CPU core (drawing the display
  would make them uneven); the frame flashes on every beat.
- Intonation: play the open string (or its 12th-fret harmonic), then the same
  string at the 12th fret; the app recognises the string, measures both notes on
  their settled part (median of 0.4–1.6 s after the pluck) and shows the
  deviation from the octave: sharp = move the saddle back (away from the neck),
  flat = forward, within 2 cents = OK. A row shows all six strings.
  `Enter` again, `r` clear all, `;`/`.` gain. The guitar must be tuned first.
- Chords: M5 moves between the fields ROOT → TYPE → SHAPE (the active one is
  yellow), PWR / B change it (roots C C# D Eb E F F# G Ab A Bb B). Types: major, `m`, `7`, `maj7`, `m7`, `sus2`, `sus4`, `dim`, `aug`, `6`, `9`,
  `add9`, `5`. The shape is drawn on a horizontal fretboard (high E string at
  the top, as seen when playing): numbers = fingers, orange = root, a green bar =
  barre, `x` = not played, a ring = open string. Next to the name: the notes of
  the chord, correctly spelled (Cm = C Eb G). Shapes: the open chord (if there
  is one), then movable barre shapes with the root on the E and A strings.
  No microphone.
- Scales: M5 moves between the fields KEY → SCALE → FRETS, PWR / B change it:
  the key, the scale (minor and major pentatonic, blues, major, minor, dorian,
  mixolydian, harmonic minor), frets 0–12 / 12–24.
  Small dots on the fretboard: orange = root, green = other notes, light blue =
  the blue note (b5). Note names are spelled for the key (F major has Bb).
- Mic test: PWR / B change the microphone gain (0–30 dB in 3 dB steps); the
  menu runs an automatic gain test or sends 3 s of raw samples to the PC. The serial console (115200 baud) prints the values 4× per second.

## Files
| Path | Content |
|------|---------|
| `src/main.cpp` | starting apps, feeding them audio, buttons, the menu and redraws |
| `src/launcher.cpp` | app carousel with icons, list of all apps |
| `src/app.h` | interface every app implements |
| `src/apps/` | the apps (`mic_test.cpp`, ...) |
| `src/services/` | `audio_in` (gapless microphone stream, gain), `settings` (NVS), `ui` (screen helpers, menu) |
| `lib/dsp/` | signal processing without hardware: levels, weighting, YIN pitch, notes, tuning, strum tuner, FFT, spectrum bands, onsets, tempo, intonation, WAV header, music theory (chord spelling and shapes) |
| `test/test_dsp/` | unit tests of `lib/dsp`, run on the PC |

## Building
PlatformIO: `pio run`, upload with `pio run -t upload`,
unit tests on the PC with `pio test -e native` (needs gcc).

The Arduino core is pinned to **3.2.0** (pioarduino platform 54.03.20), the
core the firmware was developed and measured on. `board_build.variant` is set
in `platformio.ini` because the `m5stick-c` board file still names a variant
that core 3.x no longer has.

## Notes on the StickC Plus microphone (SPM1423)
- A PDM microphone read by the ESP32's I2S peripheral (clock GPIO 0, data
  GPIO 34), powered by LDO0 of the AXP192. M5Unified removes the DC offset.
- It has no analog gain. The "microphone gain" (0–30 dB) is applied
  digitally in `audio_in` (samples are multiplied and clipped). 0 dB = the
  unscaled output of the PDM decoder (`magnification` 4 / (`over_sampling` 2 × 2)).
- The level thresholds, the default gains and the decibel meter's default
  calibration come from the Cardputer ADV (ES8311) and were **not measured on
  the StickC**. Calibrate the decibel meter (menu) against a phone app, and
  adjust the gain per app if the tuner reacts badly (too quiet / CLIP in Mic
  test).
- The buzzer (GPIO 2) is only used by the Metronome; it cannot play back
  recordings, and there is no SD card, so the Recorder app was removed.

### Pitch accuracy
Plain YIN estimates the fraction of the period with a parabola through three
points. With real sound (harmonics) that is off by up to ~0.07 samples – 3.4
cents for 440 Hz at 16 kHz. `lib/dsp/pitch.cpp` therefore measures the fraction
again over many periods (about 520 samples), which divides the error by the
number of periods. `test/test_dsp/recording_440.h` is a real recording that
guards this (plain YIN reads it +3.35 cents sharp).

The microphone needs about 1 s to warm up after power-on (returns zeros).
The microphone and the speaker share the I2S bus, so the speaker is turned
off before recording.

## License
MIT, see [LICENSE](LICENSE).
