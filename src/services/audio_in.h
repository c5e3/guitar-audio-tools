// Gapless microphone input.
//
// While started, the microphone records all the time: two small buffers are
// queued in M5.Mic (the SPM1423 PDM microphone of the M5StickC Plus); each filled buffer is copied into a FreeRTOS
// stream buffer (about 0.5 s of sound) and queued again. The main loop takes
// the samples out with read(). No sound is lost between blocks as long as the
// main loop reads faster than the microphone produces (droppedSamples() = 0).
#pragma once
#include <Arduino.h>

namespace audio_in {

// Starts recording at `sampleRate` Hz with gain `gainDb` (0..30 dB).
// Stops the buzzer (it uses the I2S peripheral too).
bool start(uint32_t sampleRate, int gainDb);
void stop();
bool running();
uint32_t sampleRate();

// Copies up to `maxCount` waiting samples into `dst`, returns how many.
// Never waits: returns 0 when nothing is ready.
size_t read(int16_t *dst, size_t maxCount);

// Microphone gain, 0...30 dB in 3 dB steps. The PDM microphone has no analog
// gain: the samples are multiplied (clipped at full scale) before read()
// returns them. 0 dB = the unscaled output of the PDM decoder.
void setGain(int db);
int gain();

// Samples thrown away because the stream buffer was full (main loop too slow).
uint32_t droppedSamples();

// Real sample rate in samples per second, measured against the CPU clock
// (0 until about 4 s after start(); gets more precise the longer it runs).
double measuredRate();

// measuredRate() / nominal rate, or 1 while not measured yet.
// Frequencies computed with the nominal rate can be multiplied by this.
// Measured on a Cardputer ADV: -15 ppm (0.03 cents), so it hardly matters,
// but it keeps the tuner exact should the StickC's clock differ more.
float rateCorrection();

}  // namespace audio_in
