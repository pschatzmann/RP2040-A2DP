#pragma once
#include "A2DPConfigRP2040.h"
#include "btstack.h"

namespace a2dp_rp2040 {

// -- Declare Sink Callback functions
void sink_hci_packet_handler(uint8_t packet_type, uint16_t channel,
                             uint8_t *packet, uint16_t size);
void sink_a2dp_packet_handler(uint8_t packet_type, uint16_t channel,
                              uint8_t *packet, uint16_t event_size);
void sink_handle_l2cap_media_data_packet(uint8_t seid, uint8_t *packet,
                                         uint16_t size);
void sink_avrcp_packet_handler(uint8_t packet_type, uint16_t channel,
                               uint8_t *packet, uint16_t size);
void sink_avrcp_controller_packet_handler(uint8_t packet_type, uint16_t channel,
                                          uint8_t *packet, uint16_t size);
void sink_avrcp_target_packet_handler(uint8_t packet_type, uint16_t channel,
                                      uint8_t *packet, uint16_t size);

void sink_handle_l2cap_media_data_packet(uint8_t seid, uint8_t *packet,
                                         uint16_t size);
void sink_avrcp_packet_handler(uint8_t packet_type, uint16_t channel,
                               uint8_t *packet, uint16_t size);
void sink_avrcp_controller_packet_handler(uint8_t packet_type, uint16_t channel,
                                          uint8_t *packet, uint16_t size);

void sink_avrcp_target_packet_handler(uint8_t packet_type, uint16_t channel,
                                      uint8_t *packet, uint16_t size);
void sink_playback_handler(int16_t *buffer, uint16_t num_audio_frames);

// -- Declare Source Callback functions

void source_a2dp_audio_timeout_handler(btstack_timer_source_t *timer);

void source_a2dp_packet_handler(uint8_t packet_type, uint16_t channel,
                                uint8_t *event, uint16_t event_size);

void source_a2dp_configure_sample_rate(int sample_rate);
void source_avrcp_controller_packet_handler(uint8_t packet_type,
                                            uint16_t channel, uint8_t *packet,
                                            uint16_t size);
void source_avrcp_target_packet_handler(uint8_t packet_type, uint16_t channel,
                                        uint8_t *packet, uint16_t size);

void source_hci_packet_handler(uint8_t packet_type, uint16_t channel,
                               uint8_t *packet, uint16_t size);

void source_avrcp_packet_handler(uint8_t packet_type, uint16_t channel,
                                 uint8_t *packet, uint16_t size);


/**
 * @brief Common A2DP functionality
 * @author Phil Schatzmann
 */
class A2DPCommonRP2040 {
public:
  //   bool setPower(bool on) {
  //     return hci_power_control(on ? HCI_POWER_ON : HCI_POWER_OFF) == 0;
  //   }
};

}
