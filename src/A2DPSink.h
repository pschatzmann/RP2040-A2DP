#pragma once

/*
 * Copyright (C) 2016 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL BLUEKITCHEN
 * GMBH OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at
 * contact@bluekitchen-gmbh.com
 *
 */
// *****************************************************************************
/* EXAMPLE_START(a2dp_sink_arduino): A2DP Sink - Receive Audio Stream and
 * Control Playback
 *
 * @text This A2DP Sink example demonstrates how to use the A2DP Sink service to
 * receive an audio data stream from a remote A2DP Source device. In addition,
 * the AVRCP Controller is used to get information on currently played media,
 * such are title, artist and album, as well as to control the playback,
 * i.e. to play, stop, repeat, etc.
 *
 * @text To test with a remote device, e.g. a mobile phone,
 * pair from the remote device with the arduino, then start playing music on the
 * remote device. Alternatively, set the device_addr_string to the Bluetooth
 * address of your remote device in the code, and call connect from the UI.
 *
 * @text For more info on BTstack audio, see our blog post
 * [A2DP Sink and Source on STM32 F4 Discovery
 * Board](http://bluekitchen-gmbh.com/a2dp-sink-and-source-on-stm32-f4-discovery-board/).
 *
 */
// *****************************************************************************
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "A2DPCommon.h"

namespace btstack_a2dp {

/**
 * @brief A2DPSink for the RP2040
 * @author Phil Schatzmann
 */

class A2DPSinkClass : public A2DPCommon {
 public:
  A2DPSinkClass() = default;

  void setOutput(AudioStream &out) {
    volume_stream.setOutput(out);
    dec_stream.setOutput(&volume_stream);
    dec_stream.setDecoder(&(get_decoder().decoder()));
  }

  void setOutput(AudioOutput &out) {
    volume_stream.setOutput(out);
    dec_stream.setOutput(&volume_stream);
    dec_stream.setDecoder(&(get_decoder().decoder()));
  }

  /// Starts A2DP Sink with the output to I2S
  bool begin(const char *name) {
    LOGI("begin: %s", name);
    a2dp_name = name;

    LOGI("Starting BTstack ...\n");
    int rc = a2dp_and_avrcp_setup();
    if (rc != 0) {
      LOGE("a2dp_and_avrcp_setup");
      return false;
    }

    // turn on!
    LOGI("Power On ...\n");
    if (hci_power_control(HCI_POWER_ON)!=0){
      LOGE("hci_power_control");
      return false;
    }

    return true;
  }

  /// Stops the processing
  void end() {
    TRACEI();
    stop();
    a2dp_sink_disconnect(get_avrcp_cid());
    dec_stream.end();
  }

  /// Sets the decoder
  void setDecoder(A2DPDecoder &dec) { p_decoder = &dec; }

  void resetDecoder() { p_decoder = &decoder_sbc; }

 protected:
  friend void sink_hci_packet_handler(uint8_t packet_type, uint16_t channel,
                                      uint8_t *packet, uint16_t size);
  friend void sink_a2dp_packet_handler(uint8_t packet_type, uint16_t channel,
                                       uint8_t *packet, uint16_t event_size);
  friend void sink_handle_l2cap_media_data_packet(uint8_t seid, uint8_t *packet,
                                                  uint16_t size);
  friend void sink_avrcp_packet_handler(uint8_t packet_type, uint16_t channel,
                                        uint8_t *packet, uint16_t size);
  friend void sink_avrcp_controller_packet_handler(uint8_t packet_type,
                                                   uint16_t channel,
                                                   uint8_t *packet,
                                                   uint16_t size);
  friend void sink_avrcp_target_packet_handler(uint8_t packet_type,
                                               uint16_t channel,
                                               uint8_t *packet, uint16_t size);

  enum stream_state_t {
    STREAM_STATE_CLOSED,
    STREAM_STATE_OPEN,
    STREAM_STATE_PLAYING,
    STREAM_STATE_PAUSED,
  };

  struct a2dp_sink_arduino_stream_endpoint_t {
    uint8_t a2dp_local_seid;
    // uint8_t media_sbc_codec_configuration[4]; ***
  } a2dp_sink_arduino_stream_endpoint;

  struct a2dp_sink_arduino_a2dp_connection_t {
    bd_addr_t addr;
    uint16_t a2dp_cid;
    uint8_t a2dp_local_seid;
    stream_state_t stream_state;
    // media_codec_configuration_sbc_t sbc_configuration; ***
  } a2dp_sink_arduino_a2dp_connection;

  struct a2dp_sink_arduino_avrcp_connection_t {
    bd_addr_t addr;
    uint16_t avrcp_cid;
    bool playing;
  } a2dp_sink_arduino_avrcp_connection;

  // local state
  A2DPDecoderSBC decoder_sbc;
  A2DPDecoder *p_decoder = &decoder_sbc;
  const char *a2dp_name = "rp2040";
  AdapterAudioStreamToAudioOutput out_print;
  EncodedAudioOutput dec_stream;
  RingBuffer<uint8_t> ring_buffer{(OPTIMAL_FRAMES_MAX + ADDITIONAL_FRAMES) *
                                  MAX_SBC_FRAME_SIZE};
  btstack_packet_callback_registration_t hci_event_callback_registration;
  uint8_t sdp_avdtp_sink_service_buffer[150];
  uint8_t sdp_avrcp_target_service_buffer[150];
  uint8_t sdp_avrcp_controller_service_buffer[200];
  uint8_t device_id_sdp_service_buffer[100];
  unsigned int sbc_frame_size;
  int media_initialized = 0;
  int audio_stream_started;
  int request_frames;
  avrcp_battery_status_t battery_status = AVRCP_BATTERY_STATUS_WARNING;
  int16_t *request_buffer;

  // local methods
  int16_t get_max_input_amplitude() override { return MAX_AMPLITUDE_RECEIVED; }

  int get_avrcp_cid() { return a2dp_sink_arduino_avrcp_connection.avrcp_cid; }

  A2DPDecoder &get_decoder() { return *p_decoder; }

  void set_playing(bool playing) override {
    is_playing = playing;
    a2dp_sink_arduino_avrcp_connection.playing = playing;
  }

  /**
   * @text The Listing MainConfiguration shows how to set up AD2P Sink and
   * AVRCP services. Besides calling init() method for each service, you'll
   * also need to register several packet handlers:
   * - sink_hci_packet_handler - handles legacy pairing, here by using fixed
   * '0000' pin code.
   * - sink_a2dp_packet_handler - handles events on stream connection status
   * (established, released), the media codec configuration, and, the status
   * of the stream itself (opened, paused, stopped).
   * - sink_handle_l2cap_media_data_packet - used to receive streaming SBC data.
   * - sink_avrcp_packet_handler - receives connect/disconnect event.
   * - sink_avrcp_controller_packet_handler - receives answers for sent AVRCP
   * commands.
   * - sink_avrcp_target_packet_handler - receives AVRCP commands, and
   * registered notifications.
   *
   * @text To announce A2DP Sink and AVRCP services, you need to create
   * corresponding SDP records and register them with the SDP service.
   *
   * @text Note, currently only the SBC codec is supported.
   */

  int a2dp_and_avrcp_setup(void) {
    LOGI("a2dp_and_avrcp_setup");
    l2cap_init();

#ifdef ENABLE_BLE
    if (isBLEEnabled()) {
      // Initialize LE Security Manager. Needed for cross-transport key
      // derivation
      sm_init();
    }
#endif
    // Initialize AVDTP Sink
    a2dp_sink_init();
    a2dp_sink_register_packet_handler(&sink_a2dp_packet_handler);
    a2dp_sink_register_media_handler(&sink_handle_l2cap_media_data_packet);

    // Create stream endpoint
    a2dp_sink_arduino_stream_endpoint_t *stream_endpoint =
        &a2dp_sink_arduino_stream_endpoint;
    A2DPDecoder &dec = get_decoder();
    avdtp_stream_endpoint_t *local_stream_endpoint =
        a2dp_sink_create_stream_endpoint(
            AVDTP_AUDIO, dec.codecType(), dec.codecCapabilities(),
            dec.codecCapabilitiesSize(), dec.config(), dec.configSize());
    if (!local_stream_endpoint) {
      LOGE("A2DP Sink: not enough memory to create local stream endpoint\n");
      return 1;
    }

    // Store stream enpoint's SEP ID, as it is used by A2DP API to identify
    // the stream endpoint
    stream_endpoint->a2dp_local_seid = avdtp_local_seid(local_stream_endpoint);

    // Initialize AVRCP service
    avrcp_init();
    avrcp_register_packet_handler(&sink_avrcp_packet_handler);

    // Initialize AVRCP Controller
    avrcp_controller_init();
    avrcp_controller_register_packet_handler(
        &sink_avrcp_controller_packet_handler);

    // Initialize AVRCP Target
    avrcp_target_init();
    avrcp_target_register_packet_handler(&sink_avrcp_target_packet_handler);

    // Initialize SDP
    sdp_init();

    // Create A2DP Sink service record and register it with SDP
    memset(sdp_avdtp_sink_service_buffer, 0,
           sizeof(sdp_avdtp_sink_service_buffer));
    a2dp_sink_create_sdp_record(sdp_avdtp_sink_service_buffer, 0x10001,
                                AVDTP_SINK_FEATURE_MASK_HEADPHONE, NULL, NULL);
    sdp_register_service(sdp_avdtp_sink_service_buffer);

    // Create AVRCP Controller service record and register it with SDP. We
    // send Category 1 commands to the media player, e.g. play/pause
    memset(sdp_avrcp_controller_service_buffer, 0,
           sizeof(sdp_avrcp_controller_service_buffer));
    uint16_t controller_supported_features =
        AVRCP_FEATURE_MASK_CATEGORY_PLAYER_OR_RECORDER;
#ifdef AVRCP_BROWSING_ENABLED
    controller_supported_features |= AVRCP_FEATURE_MASK_BROWSING;
#endif
    avrcp_controller_create_sdp_record(sdp_avrcp_controller_service_buffer,
                                       0x10002, controller_supported_features,
                                       NULL, NULL);
    sdp_register_service(sdp_avrcp_controller_service_buffer);

    // Create AVRCP Target service record and register it with SDP. We receive
    // Category 2 commands from the media player, e.g. volume up/down
    memset(sdp_avrcp_target_service_buffer, 0,
           sizeof(sdp_avrcp_target_service_buffer));
    uint16_t target_supported_features = AVRCP_FEATURE_MASK_CATEGORY_MONITOR_OR_AMPLIFIER;
    avrcp_target_create_sdp_record(sdp_avrcp_target_service_buffer, 0x10003,
                                   target_supported_features, NULL, NULL);
    sdp_register_service(sdp_avrcp_target_service_buffer);

    // Create Device ID (PnP) service record and register it with SDP
    memset(device_id_sdp_service_buffer, 0,
           sizeof(device_id_sdp_service_buffer));
    device_id_create_sdp_record(device_id_sdp_service_buffer, 0x10004,
                                DEVICE_ID_VENDOR_ID_SOURCE_BLUETOOTH,
                                BLUETOOTH_COMPANY_ID_BLUEKITCHEN_GMBH, 1, 1);
    sdp_register_service(device_id_sdp_service_buffer);

    // Set local name with a template Bluetooth address, that will be
    // automatically replaced with an actual address once it is available,
    // i.e. when BTstack boots up and starts talking to a Bluetooth module.
    gap_set_local_name(a2dp_name);

    // allot to show up in Bluetooth inquiry
    gap_discoverable_control(1);

    // Service Class: Audio, Major Device Class: Audio, Minor: Loudspeaker
    gap_set_class_of_device(0x200414);

    // allow for role switch in general and sniff mode
    gap_set_default_link_policy_settings(LM_LINK_POLICY_ENABLE_ROLE_SWITCH |
                                         LM_LINK_POLICY_ENABLE_SNIFF_MODE);

    // allow for role switch on outgoing connections - this allows A2DP
    // Source, e.g. smartphone, to become master when we re-connect to it
    gap_set_allow_role_switch(true);

    // Register for HCI events
    hci_event_callback_registration.callback = &sink_hci_packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    return 0;
  }
  /* LISTING_END */

  int media_processing_init() {
    LOGI("media_processing_init");
    if (media_initialized) return 0;

    auto &dec = get_decoder();
    dec.begin();
    auto cfg = dec.audioInfo();

    // setup decoder output
    auto cfgd = dec_stream.defaultConfig();
    cfgd.copyFrom(cfg);
    dec_stream.begin(cfgd);

    // setup volume output
    auto vcfg = volume_stream.defaultConfig();
    vcfg.copyFrom(cfg);
    vcfg.allow_boost = true;
    vcfg.volume = volume_as_float(volume_percentage);
    volume_stream.begin(vcfg);
    avrcp_volume_changed(volume_percentage);

    audio_stream_started = 0;
    media_initialized = 1;
    return 0;
  }

  void media_processing_start(void) {
    LOGI("media_processing_start");
    if (!media_initialized) return;

    dec_stream.begin();
    audio_stream_started = 1;
  }

  void media_processing_pause(void) {
    LOGI("media_processing_pause");
    if (!media_initialized) return;
    // stop audio playback
    audio_stream_started = 0;
  }

  void media_processing_close(void) {
    LOGI("media_processing_close");
    if (!media_initialized) return;
    media_initialized = 0;
    audio_stream_started = 0;
    sbc_frame_size = 0;

    dec_stream.end();
  }

  /**
   * @brief Here the audio data, are received through the
   * sink_handle_l2cap_media_data_packet callback. Currently, only the SBC media
   * codec is supported. Hence, the media data consists of the media packet
   * header and the SBC packet. The SBC frame will be stored in a ring buffer
   * for later processing (instead of decoding it to PCM right away which
   * would require a much larger buffer). If the audio stream wasn't started
   * already and there are enough SBC frames in the ring buffer, start
   * playback.
   */

  void handle_l2cap_media_data_packet(uint8_t seid, uint8_t *packet,
                                                 uint16_t size) {
    LOGI("handle_l2cap_media_data_packet");
    int pos = 0;
    //   avdtp_media_packet_header_t media_header;
    avdtp_media_packet_header_t media_header;
    if (!read_media_data_header(packet, size, &pos, &media_header)) return;
    avdtp_sbc_codec_header_t sbc_header;
    if (!read_sbc_header(packet, size, &pos, &sbc_header)) return;

    // store sbc frame size for buffer management
    sbc_frame_size = (size - pos) / sbc_header.num_frames;
    dec_stream.write(packet + pos, size - pos);
  }

  int read_sbc_header(uint8_t *packet, int size, int *offset,
                      avdtp_sbc_codec_header_t *sbc_header) {
    LOGI("read_sbc_header");
    int sbc_header_len = 12;  // without crc
    int pos = *offset;

    if (size - pos < sbc_header_len) {
      LOGW("Not enough data to read SBC header, expected %d, received %d",
           sbc_header_len, size - pos);
      return 0;
    }

    sbc_header->fragmentation = get_bit16(packet[pos], 7);
    sbc_header->starting_packet = get_bit16(packet[pos], 6);
    sbc_header->last_packet = get_bit16(packet[pos], 5);
    sbc_header->num_frames = packet[pos] & 0x0f;
    pos++;
    *offset = pos;
    return 1;
  }

  int read_media_data_header(uint8_t *packet, int size, int *offset,
                             avdtp_media_packet_header_t *media_header) {
    LOGI("read_media_data_header");
    int media_header_len = 12;  // without crc
    int pos = *offset;

    if (size - pos < media_header_len) {
      LOGW(
          "Not enough data to read media packet header, expected %d, "
          "received "
          "%d",
          media_header_len, size - pos);
      return 0;
    }

    media_header->version = packet[pos] & 0x03;
    media_header->padding = get_bit16(packet[pos], 2);
    media_header->extension = get_bit16(packet[pos], 3);
    media_header->csrc_count = (packet[pos] >> 4) & 0x0F;
    pos++;

    media_header->marker = get_bit16(packet[pos], 0);
    media_header->payload_type = (packet[pos] >> 1) & 0x7F;
    pos++;

    media_header->sequence_number = big_endian_read_16(packet, pos);
    pos += 2;

    media_header->timestamp = big_endian_read_32(packet, pos);
    pos += 4;

    media_header->synchronization_source = big_endian_read_32(packet, pos);
    pos += 4;
    *offset = pos;
    return 1;
  }

  virtual void avrcp_packet_handler(uint8_t packet_type, uint16_t channel,
                                       uint8_t *packet, uint16_t size) {
    LOGI("avrcp_packet_handler");
    UNUSED(channel);
    UNUSED(size);
    uint16_t local_cid;
    uint8_t status;
    bd_addr_t address;

    a2dp_sink_arduino_avrcp_connection_t *connection =
        &a2dp_sink_arduino_avrcp_connection;

    if (packet_type != HCI_EVENT_PACKET) return;
    if (hci_event_packet_get_type(packet) != HCI_EVENT_AVRCP_META) return;

    switch (packet[2]) {
      case AVRCP_SUBEVENT_CONNECTION_ESTABLISHED: {
        local_cid = avrcp_subevent_connection_established_get_avrcp_cid(packet);
        status = avrcp_subevent_connection_established_get_status(packet);
        if (status != ERROR_CODE_SUCCESS) {
          LOGE("AVRCP: Connection failed: status 0x%02x", status);
          connection->avrcp_cid = 0;
          return;
        }

        connection->avrcp_cid = local_cid;
        avrcp_subevent_connection_established_get_bd_addr(packet, address);
        LOGI("AVRCP: Connected to %s, cid 0x%02x\n", bd_addr_to_str(address),
             connection->avrcp_cid);

        avrcp_target_support_event(connection->avrcp_cid,
                                   AVRCP_NOTIFICATION_EVENT_VOLUME_CHANGED);
        avrcp_target_support_event(
            connection->avrcp_cid,
            AVRCP_NOTIFICATION_EVENT_BATT_STATUS_CHANGED);
        avrcp_target_battery_status_changed(connection->avrcp_cid,
                                            battery_status);

        // automatically enable notifications
        avrcp_controller_enable_notification(
            connection->avrcp_cid,
            AVRCP_NOTIFICATION_EVENT_PLAYBACK_STATUS_CHANGED);
        avrcp_controller_enable_notification(
            connection->avrcp_cid, AVRCP_NOTIFICATION_EVENT_VOLUME_CHANGED);
        avrcp_controller_enable_notification(
            connection->avrcp_cid,
            AVRCP_NOTIFICATION_EVENT_NOW_PLAYING_CONTENT_CHANGED);
        avrcp_controller_enable_notification(
            connection->avrcp_cid, AVRCP_NOTIFICATION_EVENT_TRACK_CHANGED);
        return;
      }

      case AVRCP_SUBEVENT_CONNECTION_RELEASED:
        LOGI("AVRCP: Channel released: cid 0x%02x",
             avrcp_subevent_connection_released_get_avrcp_cid(packet));
        connection->avrcp_cid = 0;
        return;
      default:
        break;
    }
  }

  virtual void avrcp_target_packet_handler(uint8_t packet_type,
                                              uint16_t channel, uint8_t *packet,
                                              uint16_t size) {
    LOGI("avrcp_target_packet_handler");
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) return;
    if (hci_event_packet_get_type(packet) != HCI_EVENT_AVRCP_META) return;

    uint8_t volume;
    char const *button_state;
    avrcp_operation_id_t operation_id;

    switch (packet[2]) {
      case AVRCP_SUBEVENT_NOTIFICATION_VOLUME_CHANGED:
        volume = avrcp_subevent_notification_volume_changed_get_absolute_volume(
            packet);
        volume_percentage = volume_to_percent(volume);
        LOGI("AVRCP Target    : Volume set to %d%% (%d)", volume_percentage,
             volume);
        avrcp_volume_changed(volume);
        break;

      case AVRCP_SUBEVENT_OPERATION:
        operation_id =
            (avrcp_operation_id_t)avrcp_subevent_operation_get_operation_id(
                (const uint8_t *)packet);
        button_state = avrcp_subevent_operation_get_button_pressed(packet) > 0
                           ? "PRESS"
                           : "RELEASE";
        switch (operation_id) {
          case AVRCP_OPERATION_ID_VOLUME_UP:
            LOGI("AVRCP Target    : VOLUME UP (%s)", button_state);
            break;
          case AVRCP_OPERATION_ID_VOLUME_DOWN:
            LOGI("AVRCP Target    : VOLUME DOWN (%s)", button_state);
            break;
          default:
            return;
        }
        break;
      default:
        LOGI("AVRCP Target    : Event 0x%02x is not parsed", packet[2]);
        break;
    }
  }

  virtual void hci_packet_handler(uint8_t packet_type, uint16_t channel,
                                     uint8_t *packet, uint16_t size) {
    LOGI("hci_packet_handler");
    UNUSED(channel);
    UNUSED(size);
    if (packet_type != HCI_EVENT_PACKET) return;
    if (hci_event_packet_get_type(packet) == HCI_EVENT_PIN_CODE_REQUEST) {
      bd_addr_t address;
      LOGI("Pin code request - using '0000'");
      hci_event_pin_code_request_get_bd_addr(packet, address);
      gap_pin_code_response(address, "0000");
    }
  }

  virtual void a2dp_packet_handler(uint8_t packet_type, uint16_t channel,
                                      uint8_t *packet, uint16_t size) {
    LOGI("a2dp_packet_handler");
    UNUSED(channel);
    UNUSED(size);
    bd_addr_t address;
    uint8_t status;
    auto &dec = get_decoder();

    if (packet_type != HCI_EVENT_PACKET) return;
    if (hci_event_packet_get_type(packet) != HCI_EVENT_A2DP_META) return;

    a2dp_sink_arduino_a2dp_connection_t *a2dp_conn =
        &a2dp_sink_arduino_a2dp_connection;

    switch (packet[2]) {
      case A2DP_SUBEVENT_SIGNALING_MEDIA_CODEC_OTHER_CONFIGURATION:
        LOGI("A2DP  Sink      : Received non SBC codec - not implemented");
        break;
      case A2DP_SUBEVENT_SIGNALING_MEDIA_CODEC_SBC_CONFIGURATION: {
        dec.setValues(packet, size);
        break;
      }
      case A2DP_SUBEVENT_STREAM_ESTABLISHED:
        a2dp_subevent_stream_established_get_bd_addr(packet, a2dp_conn->addr);

        status = a2dp_subevent_stream_established_get_status(packet);
        if (status != ERROR_CODE_SUCCESS) {
          LOGE("A2DP  Sink      : Streaming connection failed, status 0x%02x",
               status);
          break;
        }

        a2dp_conn->a2dp_cid =
            a2dp_subevent_stream_established_get_a2dp_cid(packet);
        a2dp_conn->stream_state = STREAM_STATE_OPEN;

        LOGI(
            "A2DP  Sink      : Streaming connection is established, address "
            "%s, "
            "cid 0x%02X, local seid %d",
            bd_addr_to_str(address), a2dp_conn->a2dp_cid,
            a2dp_conn->a2dp_local_seid);
        break;

#ifdef ENABLE_AVDTP_ACCEPTOR_EXPLICIT_START_STREAM_CONFIRMATION
      case A2DP_SUBEVENT_START_STREAM_REQUESTED:
        LOGI(
            "A2DP  Sink      : Explicit Accept to start stream, local_seid "
            "0x%02x",
            a2dp_subevent_start_stream_requested_get_local_seid(packet));
        // ps
        a2dp_sink_start_stream_accept(a2dp_conn->a2dp_cid,
                                      a2dp_conn->a2dp_local_seid);
        break;
#endif
      case A2DP_SUBEVENT_STREAM_STARTED: {
        LOGI("A2DP  Sink      : Stream started");
        auto &dec = get_decoder();
        a2dp_conn->stream_state = STREAM_STATE_PLAYING;
        if (dec.isReconfigure()) {
          media_processing_close();
        }
        // prepare media processing
        media_processing_init();
        // audio stream is started when buffer reaches minimal level
      } break;

      case A2DP_SUBEVENT_STREAM_SUSPENDED:
        LOGI("A2DP  Sink      : Stream paused");
        a2dp_conn->stream_state = STREAM_STATE_PAUSED;
        media_processing_pause();
        break;

      case A2DP_SUBEVENT_STREAM_RELEASED:
        LOGI("A2DP  Sink      : Stream released");
        a2dp_conn->stream_state = STREAM_STATE_CLOSED;
        media_processing_close();
        break;

      case A2DP_SUBEVENT_SIGNALING_CONNECTION_RELEASED:
        LOGI("A2DP  Sink      : Signaling connection released");
        a2dp_conn->a2dp_cid = 0;
        media_processing_close();
        break;

      default:
        break;
    }
  }

} A2DPSink;

// -- Implement Callback functions which forward calls to A2DPSink

void sink_hci_packet_handler(uint8_t packet_type, uint16_t channel,
                             uint8_t *packet, uint16_t size) {
  A2DPSink.hci_packet_handler(packet_type, channel, packet, size);
}

void sink_a2dp_packet_handler(uint8_t packet_type, uint16_t channel,
                              uint8_t *packet, uint16_t event_size) {
  A2DPSink.a2dp_packet_handler(packet_type, channel, packet,
                                          event_size);
}

void sink_handle_l2cap_media_data_packet(uint8_t seid, uint8_t *packet,
                                         uint16_t size) {
  A2DPSink.handle_l2cap_media_data_packet(seid, packet, size);
}

void sink_avrcp_packet_handler(uint8_t packet_type, uint16_t channel,
                               uint8_t *packet, uint16_t size) {
  A2DPSink.avrcp_packet_handler(packet_type, channel, packet, size);
}

void sink_avrcp_controller_packet_handler(uint8_t packet_type, uint16_t channel,
                                          uint8_t *packet, uint16_t size) {
  A2DPSink.avrcp_controller_packet_handler(packet_type, channel, packet,
                                                 size);
}

void sink_avrcp_target_packet_handler(uint8_t packet_type, uint16_t channel,
                                      uint8_t *packet, uint16_t size) {
  A2DPSink.avrcp_target_packet_handler(packet_type, channel, packet,
                                                  size);
}

}  // namespace btstack_a2dp