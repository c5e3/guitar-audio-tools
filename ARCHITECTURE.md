# Guitar Audio Tools – architecture

One firmware for the M5Stack M5StickC Plus 1.1 containing several audio apps
(ported from the Cardputer ADV version).
A shared core does the hard work; each app is a thin layer on top of it.

## Layers

```
┌──────────────────────── APPS (one file each) ─────────────────────────┐
│ Decibel │ Tuner │ Strum tuner │ Spectrum │ BPM │ Metronome │ Intonation │ Chords │ Scales │
└────────────────────────────────────────────────────────────────────────┘
┌──────────── DSP library (lib/dsp: pure C++, no hardware) ─────────────┐
│ level (RMS, dB SPL, A-weighting, Leq) · pitch (YIN) · fft · bands      │
│ onset/beat (BPM) · notes (Hz -> note, cents) · polytune                │
└────────────────────────────────────────────────────────────────────────┘
┌──────────── Services (src/services) ──────────────────────────────────┐
│ audio_in (gapless sample stream) · settings (NVS: calibration, A4...)  │
│ ui (canvas, header, widgets, menu)                                     │
└────────────────────────────────────────────────────────────────────────┘
┌──────────── Hardware (M5Unified library) ─────────────────────────────┐
│ display · 3 buttons (M5, B, PWR) · PDM microphone · buzzer             │
└────────────────────────────────────────────────────────────────────────┘
```

Rules:
- **DSP code never includes Arduino or M5 headers.** It only gets arrays of
  samples, so it can be unit-tested on a PC (`pio test -e native`) with
  synthetic signals (e.g. a 440 Hz sine must be measured as 440.0 Hz).
- **Apps never talk to the hardware directly**: they get audio from
  `audio_in`, draw through `ui` and store through `settings`.
- Only **one app runs at a time**. Big buffers (FFT, recordings) are
  allocated in `enter()` and freed in `exit()` – the ESP32-S3 in the
  StickC Plus has no PSRAM (about 320 KB RAM).

## App interface

Every app implements `App` (`src/app.h`):

| Method | Called |
|--------|--------|
| `name()` | menu title |
| `sampleRate()`, `micGain()` | before `enter()`, to set up the microphone |
| `enter()` / `exit()` | when the app is opened / closed |
| `process(samples, count)` | for every chunk of new audio (gapless stream) |
| `draw(canvas)` | about 30× per second |
| `onKey(key)` | on a button press or menu entry (M5 = `enter`, B = `/`, PWR = `,`) |
| `menu(items)` | the app's menu entries (hold M5); each sends a key to `onKey` |

Adding an app = one new file in `src/apps/`, its declaration in
`src/apps/apps.h` and one line (title, description, icon) in `src/launcher.cpp`.

## Launcher and buttons
A carousel like the Bruce firmware main menu: one app per screen with a big
icon drawn from simple shapes, PWR / B browse, M5 opens. The last opened app
is remembered in NVS.

main.cpp turns the three buttons into `Key`s: M5 click = `enter`, B = `/`,
PWR click = `,`. Holding M5 opens a menu built from the app's `menu()` plus
Help and Back to the apps; holding PWR returns to the launcher. So the apps
keep their original key handling: a menu entry simply sends the key the
Cardputer keyboard used to send (`;` gain +, `m` mode ...).

## Audio input

`audio_in` keeps the microphone recording all the time while an app is open:
two buffers are queued with `M5Cardputer.Mic.record()`; whenever one is
filled, the Mic release callback copies it into a FreeRTOS stream buffer and
queues it again. The main loop reads the stream buffer and passes the samples
to the app. No sound is lost between blocks (needed for BPM and recording).

- Arduino core **3.2.0** is required (see README).
- The PDM microphone has no analog gain: `audio_in` multiplies the samples
  (0–30 dB, `setGain()`), in the Mic buffer callback.
- An app that plays a sound (buzzer) does not use the microphone
  (`sampleRate()` 0); `audio_in::start()` stops the buzzer.

## Shared parts

| Part | Used by |
|------|---------|
| pitch detection (YIN) + needle gauge | tuner, intonation |
| FFT | spectrum, strum tuner, BPM |
| level (dB SPL, Leq, A-weighting) | decibel meter, monitor |

## Roadmap

| # | Step | Why in this order |
|---|------|-------------------|
| 0 | Skeleton: menu, gapless audio input, app interface, settings, PC tests | everything else builds on it |
| 1 | Decibel meter: dB SPL, calibration, max, Leq, A-weighting | simplest app, validates the skeleton |
| 2 | Tuner: needle gauge ±50 cents in 5-cent steps, green centre ±3 cents, big note name | YIN already works; also verify the real sample rate (440 Hz reads 441.0 Hz) |
| 3 | Metronome (instead of a vocal intonation trainer) | needs the speaker, not the microphone: an app must be able to run without `audio_in` |
| 4 | Spectrum analyser: green/yellow/red bars with peak hold | introduces FFT |
| 5 | Strum tuner (all strings at once) | FFT + tuner knowledge; needs fine frequency resolution (E2–A2 are 28 Hz apart) |
| 6 | BPM detector (listening and tap tempo) | FFT/onsets, gapless audio |
| 7 | Recorder to SD card | SD + WAV |
| 8 | (dropped: night/snoring monitor – the firmware stays focused on musicians) | |
| 9 | Chords, then Scales (instead of a vocal trainer) | no audio: music theory in `lib/dsp/theory.*` (tested on the PC), shared fretboard drawing |

## Quality
- Unit tests for all DSP code: `pio test -e native`.
- Code review (`/code-review`) before each commit; security review once
  networking (WiFi, web access to recordings) is added.
- One git commit per finished step.
