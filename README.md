# RP2040 A2DP
The RP2040 W is a microcontroller that provides a Bluetooth A2DP API which can be used to receive and process __audio data__ e.g. from your Mobile Phone. The output is a PCM data stream decoded from SBC format. 

The goal of this project is to make the __full A2DP functionality available for the RP2040__. 

![RP2040W](https://www.pschatzmann.ch/wp-content/uploads/2023/03/raspberry-pi-pico-w-1.jpg)

I am currently already providing an [A2DP library for the ESP32](https://github.com/pschatzmann/ESP32-A2DP). 

The RP2040 W uses the [bluekitchen btstack](https://github.com/bluekitchen/btstack). There, you can find examples for a [A2DP source](https://github.com/bluekitchen/btstack/blob/master/example/a2dp_source_demo.c) and [A2DP sink](https://github.com/bluekitchen/btstack/blob/master/example/a2dp_sink_demo.c).

However, it seems that no SBC codec is provided with the RP2040.  

I changed the examples to use C++ classes. I also added a proper integration with the [Arduino Audio Tools](https://github.com/pschatzmann/arduino-audio-tools) so that you can use the

- Audio Sources (for the A2DP Source) 
- Audio Sinks (for the A2DP Sinc)
- SBC codec 

Currently I am supporting only SBC, but it should be quite easy to extend the functionality to support other codecs as well. 


## Documentation

- [Classes](https://pschatzmann.github.io/RP2040-A2DP/docs/html/annotated.html)
- Further information about the RP2040 Bluetooth integration can be found in [the pico documentation](https://arduino-pico.readthedocs.io/en/latest/bluetooth.html).


## Dependencies

You need to install the following projects:

- https://github.com/pschatzmann/RP2040-A2DP
- https://github.com/pschatzmann/arduino-audio-tools 
- https://github.com/pschatzmann/arduino-libsbc

