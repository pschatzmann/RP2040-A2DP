#include "A2DP_RP2040.h"

I2SStream out;

void setup() {
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);

  A2DPSink.begin(out, "rp2040");
}

void loop() {}