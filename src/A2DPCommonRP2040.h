#pragma once
#include "A2DPConfigRP2040.h"
#include "btstack.h"
#include "bluetooth.h"
#include "btstack_defines.h"
#include <pico/cyw43_arch.h>

#include "AudioTools.h"
#include "AudioCodecs/CodecSBC.h"

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

// void sink_playback_handler(int16_t *buffer, uint16_t num_audio_frames);

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
class A2DPCommon {
public:

  void lockBluetooth() {
    async_context_acquire_lock_blocking(cyw43_arch_async_context());
  }

  void unlockBluetooth() {
    async_context_release_lock(cyw43_arch_async_context());
  }

  /// avrcp play
  bool play() {
    TRACEI();
    return 0 ==
           avrcp_controller_play(get_avrcp_cid());
  }

  /// avrcp stop
  bool stop() {
    TRACEI();
    return 0 ==
           avrcp_controller_stop(get_avrcp_cid());
  }

  /// avrcp pause
  bool pause() {
    TRACEI();
    return 0 ==
           avrcp_controller_pause(get_avrcp_cid());
  }

  /// avrcp forward
  bool next() {
    TRACEI();
    return 0 == avrcp_controller_forward(
                    get_avrcp_cid());
  }

  /// avrcp backward
  bool previous() {
    TRACEI();
    return 0 == avrcp_controller_backward(
                    get_avrcp_cid());
  }

  /// avrcp fast_forwar
  bool fastForward(bool start) {
    TRACEI();
    if (start) {
      return 0 == avrcp_controller_press_and_hold_fast_forward(
                      get_avrcp_cid());
    } else {
      return 0 == avrcp_controller_release_press_and_hold_cmd(
                      get_avrcp_cid());
    }
  }

  /// avrcp rewind
  bool rewind(bool start) {
    TRACEI();
    if (start) {
      return 0 == avrcp_controller_press_and_hold_rewind(
                      get_avrcp_cid());

    } else {
      return 0 == avrcp_controller_release_press_and_hold_cmd(
                      get_avrcp_cid());
    }
  }


protected:
  virtual int get_avrcp_cid() = 0;
};

} // namespace a2dp_rp2040
