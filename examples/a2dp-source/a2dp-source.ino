#include "AudioTools.h"
#include "BTstack_A2DP.h"

SineWaveGenerator<int16_t> sineWave(32000);
GeneratedSoundStream<int16_t> in(sineWave);

void setup() {
  Serial.begin(115200);
  waitFor(Serial);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);

  A2DPSource.setVolume(50);
  A2DPSource.begin(in);
}

void loop() {}