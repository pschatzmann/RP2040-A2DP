#include "AudioTools.h"
#include "A2DP_RP2040.h"

SineWaveGenerator<int16_t> sineWave(32000);
GeneratedSoundStream<int16_t> in(sineWave);

void setup() {
  Serial.begin(115200);
  waitFor(Serial);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);

  A2DPSource.begin(in);
}

void loop() {}