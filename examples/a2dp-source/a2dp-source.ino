#include "AudioTools.h"
#include "A2DP_RP2040.h"

SineWaveGenerator<int16_t> sineWave(32000);               
GeneratedSoundStream<int16_t> in(sineWave);               

void setup(){
    A2DPSource.begin(in);
}

void loop() {

}