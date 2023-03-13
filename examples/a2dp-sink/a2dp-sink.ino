#include "A2DP_RP2040.h"

I2SStream out;

void setup() { A2DPSink.begin(out, "rp2040"); }

void loop() {}