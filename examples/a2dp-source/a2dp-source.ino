#include "AudioTools.h"
#include "A2DP_RP2040.h"

SineWaveGenerator<int16_t> sineWave(32000);
GeneratedSoundStream<int16_t> in(sineWave);

void setup() {
  Serial.begin(115200);
  waitFor(Serial);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);

  // make sure that the input matches the requirements
  auto cfg = in.defaultConfig();
  cfg.sample_rate = 44100;
  cfg.channels = 2;
  cfg.bits_per_sample = 16;
  in.begin(cfg);

  A2DPSource.begin(in);
}

void loop() {}