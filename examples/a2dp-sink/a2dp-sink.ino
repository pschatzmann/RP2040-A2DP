#include "AudioTools.h"
#include "BTstack_A2DP.h"


I2SStream out;

void setup() {
  Serial.begin(115200);
  while(!Serial);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);

  A2DPSink.setOutput(out);
  A2DPSink.setVolume(50);
  A2DPSink.begin("rp2040");
}

void loop() {}