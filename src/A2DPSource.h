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
/* EXAMPLE_START(a2dp_source_arduino): A2DP Source - Stream Audio and Control
 * Volume
 *
 * @text This A2DP Source example arduinonstrates how to send an audio data
 * stream to a remote A2DP Sink device and how to switch between two audio data
 * sources. In addition, the AVRCP Target is used to answer queries on currently
 * played media, as well as to handle remote playback control, i.e. play, stop,
 * repeat, etc.
 *
 * @text To test with a remote device, e.g. a Bluetooth speaker,
 * set the device_addr_string to the Bluetooth address of your
 * remote device in the code, and use the UI to connect and start playback.
 *
 * @text For more info on BTstack audio, see our blog post
 * [A2DP Sink and Source on STM32 F4 Discovery
 * Board](http://bluekitchen-gmbh.com/a2dp-sink-and-source-on-stm32-f4-discovery-board/).
 *
 */
// *****************************************************************************
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "A2DPCommon.h"

namespace btstack_a2dp {

// -- Declare Source Callback functions

extern "C" void source_a2dp_audio_timeout_handler(btstack_timer_source_t *timer);

extern "C" void source_a2dp_packet_handler(uint8_t packet_type, uint16_t channel,
                                uint8_t *event, uint16_t event_size);

extern "C" void source_a2dp_configure_sample_rate(int sample_rate);
extern "C" void source_avrcp_controller_packet_handler(uint8_t packet_type,
                                            uint16_t channel, uint8_t *packet,
                                            uint16_t size);
extern "C" void source_avrcp_target_packet_handler(uint8_t packet_type, uint16_t channel,
                                        uint8_t *packet, uint16_t size);

extern "C" void source_hci_packet_handler(uint8_t packet_type, uint16_t channel,
                               uint8_t *packet, uint16_t size);

extern "C" void source_avrcp_packet_handler(uint8_t packet_type, uint16_t channel,
                                 uint8_t *packet, uint16_t size);


/**
 * @brief A2DPSource for the RP2040
 * @author Phil Schatzmann
 */
class A2DPSourceClass : public A2DPCommon {
 public:
  bool begin(Stream &in) { return begin(in, nullptr); }

  bool begin(AudioStream &in) { return begin(in, nullptr); }

  bool begin(AudioStream &in, const char *name) {
    TRACEI();
    p_input = &in;
    Stream &stream = in;
    return begin(stream, name);
  }

  bool begin(Stream &in, const char *name) {
    TRACEI();
    volume_stream.setStream(in);
    remote_name = name;
    // setup output chain: in -> volume_stream -> encoder_stream -> queue
    encoder_stream.setOutput(&media_tracker.queue);
    encoder_stream.setEncoder(&(get_encoder().encoder()));
    setupTrack();
    int err = a2dp_source_and_avrcp_services_init();
    // hci_power_control(HCI_POWER_ON);
    setPower(true);
    return err = 0;
  }

  /// Defines the encoder. Set a value if you do not intend to use the default
  /// SBC encoder!
  void setEncoder(A2DPEncoder &enc) { p_encoder = &enc; }

  /// Resets the conder to use the SBC encoder
  void resetEncoder() { p_encoder = &encoder_sbc; }

  /// Provides access to the track information (to read or update)
  avrcp_track_t &track() { return track_info; }

 protected:
  friend void source_a2dp_audio_timeout_handler(btstack_timer_source_t *timer);

  friend void source_a2dp_packet_handler(uint8_t packet_type, uint16_t channel,
                                         uint8_t *event, uint16_t event_size);

  friend void source_a2dp_configure_sample_rate(int sample_rate);
  friend void source_avrcp_controller_packet_handler(uint8_t packet_type,
                                                     uint16_t channel,
                                                     uint8_t *packet,
                                                     uint16_t size);
  friend void source_avrcp_target_packet_handler(uint8_t packet_type,
                                                 uint16_t channel,
                                                 uint8_t *packet,
                                                 uint16_t size);

  friend void source_hci_packet_handler(uint8_t packet_type, uint16_t channel,
                                        uint8_t *packet, uint16_t size);

  friend void source_avrcp_packet_handler(uint8_t packet_type, uint16_t channel,
                                          uint8_t *packet, uint16_t size);

  struct a2dp_media_sending_context_t {
    uint16_t a2dp_cid;
    uint8_t local_seid;
    uint8_t remote_seid;
    uint8_t stream_opened;
    uint16_t avrcp_cid;
    btstack_timer_source_t audio_timer;
    int max_media_payload_size;
    RingBuffer<uint8_t> rb{SBC_STORAGE_SIZE};
    QueueStream<uint8_t> queue{rb};
    bool is_streaming = false;
    bool sbc_is_busy = false;
  } media_tracker;

  struct avrcp_play_status_info_t {
    uint8_t track_id[8];
    uint32_t song_length_ms;
    avrcp_playback_status_t status;
    uint32_t song_position_ms;  // 0xFFFFFFFF if not supported
  };

  // State
  A2DPEncoderSBC encoder_sbc;
  A2DPEncoder *p_encoder = &encoder_sbc;
  Stream *p_in = nullptr;
  EncodedAudioStream encoder_stream;
  AudioStream *p_input = nullptr;
  avrcp_track_t track_info;
  bool is_streams_opened = false;
  btstack_packet_callback_registration_t hci_event_callback_registration;
  const int A2DP_SOURCE_arduino_INQUIRY_DURATION_1280MS = 12;
  const char *device_addr_string = "00:21:3C:AC:F7:38";
  const char *remote_name = nullptr;
  bd_addr_t device_addr;
  bool scan_active;
  uint8_t sdp_a2dp_source_service_buffer[150];
  uint8_t sdp_avrcp_target_service_buffer[200];
  uint8_t sdp_avrcp_controller_service_buffer[200];
  uint8_t device_id_sdp_service_buffer[100];
  int current_sample_rate = 44100;
  int new_sample_rate = 44100;
  int current_track_index;
  int data_source = 0;
  const int track_count = 1;
  avrcp_play_status_info_t play_info;

  // Methods

  int16_t get_max_input_amplitude() override { return MAX_AMPLITUDE_INPUT; }

  virtual int get_avrcp_cid() { return media_tracker.a2dp_cid; }

  A2DPEncoder &get_encoder() { return *p_encoder; }

  /**
   * @text The Listing MainConfiguration shows how to setup AD2P Source and
   * AVRCP services. Besides calling init() method for each service, you'll also
   * need to register several packet handlers:
   * - source_hci_packet_handler - handles legacy pairing, here by using fixed
   * '0000' pin code.
   * - source_a2dp_packet_handler - handles events on stream connection status
   * (established, released), the media codec configuration, and, the commands
   * on stream itself (open, pause, stopp).
   * - source_avrcp_packet_handler - receives connect/disconnect event.
   * - source_avrcp_controller_packet_handler - receives answers for sent AVRCP
   * commands.
   * - source_avrcp_target_packet_handler - receives AVRCP commands, and
   * registered notifications.
   * @text To announce A2DP Source and AVRCP services, you need to create
   * corresponding SDP records and register them with the SDP service.
   */

  // /* LISTING_START(MainConfiguration): Setup Audio Source and AVRCP Target
  //  * services */

  int a2dp_source_and_avrcp_services_init(void) {
    TRACED();

    // Request role change on reconnecting headset to always use them in slave
    // mode
    hci_set_master_slave_policy(0);
    // enabled EIR
    hci_set_inquiry_mode(INQUIRY_MODE_RSSI_AND_EIR);

    l2cap_init();

    if (isBLEEnabled()) {
      // Initialize LE Security Manager. Needed for cross-transport key
      // derivation
      sm_init();
    }

    // Initialize  A2DP Source
    a2dp_source_init();
    a2dp_source_register_packet_handler(&source_a2dp_packet_handler);

    // Create stream endpoint
    A2DPEncoder &enc = get_encoder();
    avdtp_stream_endpoint_t *local_stream_endpoint =
        a2dp_source_create_stream_endpoint(
            AVDTP_AUDIO, enc.codecType(), enc.codecCapabilities(),
            enc.codecCapabilitiesSize(), enc.config(), enc.configSize());
    if (!local_stream_endpoint) {
      LOGE("A2DP Source: not enough memory to create local stream endpoint");
      return 1;
    }

    // Store stream enpoint's SEP ID, as it is used by A2DP API to indentify the
    // stream endpoint
    media_tracker.local_seid = avdtp_local_seid(local_stream_endpoint);
    avdtp_source_register_delay_reporting_category(media_tracker.local_seid);

    // Initialize AVRCP Service
    avrcp_init();
    avrcp_register_packet_handler(&source_avrcp_packet_handler);
    // Initialize AVRCP Target
    avrcp_target_init();
    avrcp_target_register_packet_handler(&source_avrcp_target_packet_handler);

    // Initialize AVRCP Controller
    avrcp_controller_init();
    avrcp_controller_register_packet_handler(
        &source_avrcp_controller_packet_handler);

    // Initialize SDP,
    sdp_init();

    // Create A2DP Source service record and register it with SDP
    memset(sdp_a2dp_source_service_buffer, 0,
           sizeof(sdp_a2dp_source_service_buffer));
    a2dp_source_create_sdp_record(sdp_a2dp_source_service_buffer, 0x10001,
                                  AVDTP_SOURCE_FEATURE_MASK_PLAYER, NULL, NULL);
    sdp_register_service(sdp_a2dp_source_service_buffer);

    // Create AVRCP Target service record and register it with SDP. We receive
    // Category 1 commands from the headphone, e.g. play/pause
    memset(sdp_avrcp_target_service_buffer, 0,
           sizeof(sdp_avrcp_target_service_buffer));
    uint16_t supported_features =
        AVRCP_FEATURE_MASK_CATEGORY_PLAYER_OR_RECORDER;
#ifdef AVRCP_BROWSING_ENABLED
    supported_features |= AVRCP_FEATURE_MASK_BROWSING;
#endif
    avrcp_target_create_sdp_record(sdp_avrcp_target_service_buffer, 0x10002,
                                   supported_features, NULL, NULL);
    sdp_register_service(sdp_avrcp_target_service_buffer);

    // Create AVRCP Controller service record and register it with SDP. We send
    // Category 2 commands to the headphone, e.g. volume up/down
    memset(sdp_avrcp_controller_service_buffer, 0,
           sizeof(sdp_avrcp_controller_service_buffer));
    uint16_t controller_supported_features =
        AVRCP_FEATURE_MASK_CATEGORY_MONITOR_OR_AMPLIFIER;
    avrcp_controller_create_sdp_record(sdp_avrcp_controller_service_buffer,
                                       0x10003, controller_supported_features,
                                       NULL, NULL);
    sdp_register_service(sdp_avrcp_controller_service_buffer);

    // Register Device ID (PnP) service SDP record
    memset(device_id_sdp_service_buffer, 0,
           sizeof(device_id_sdp_service_buffer));
    device_id_create_sdp_record(device_id_sdp_service_buffer, 0x10004,
                                DEVICE_ID_VENDOR_ID_SOURCE_BLUETOOTH,
                                BLUETOOTH_COMPANY_ID_BLUEKITCHEN_GMBH, 1, 1);
    sdp_register_service(device_id_sdp_service_buffer);

    // Set local name with a template Bluetooth address, that will be
    // automatically replaced with a actual address once it is available, i.e.
    // when BTstack boots up and starts talking to a Bluetooth module.
    gap_set_local_name("A2DP Source 00:00:00:00:00:00");
    gap_discoverable_control(1);
    gap_set_class_of_device(0x200408);

    // Register for HCI events.
    hci_event_callback_registration.callback = &source_hci_packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    source_a2dp_configure_sample_rate(current_sample_rate);

    // Parse human readable Bluetooth address.
    sscanf_bd_addr(device_addr_string, device_addr);

    return 0;
  }
  /* LISTING_END */

  void source_a2dp_configure_sample_rate(int sample_rate) {
    LOGI("source_a2dp_configure_sample_rate: %d", sample_rate);
    bool is_reopen = false;
    if (is_streams_opened && sample_rate != current_sample_rate) {
      is_reopen = true;
    }

    current_sample_rate = sample_rate;

    // when the sample rate has changed we need to reopen
    if (is_reopen) {
      open_audio_streams();
    }
  }
  void open_audio_streams() {
    TRACEI();
    // set encoder parameters
    get_encoder().begin();

    // setup ecoder_stream
    auto cfg = encoder_stream.defaultConfig();
    cfg.sample_rate = current_sample_rate;
    cfg.channels = NUM_CHANNELS;
    encoder_stream.begin(cfg);

    // setup volume input
    auto vcfg = volume_stream.defaultConfig();
    vcfg.copyFrom(cfg);
    vcfg.volume = 0.01f * volume_percentage;
    volume_stream.begin(vcfg);

    // configure input if possible
    if (p_input != nullptr) {
      p_input->setAudioInfo(cfg);
    }

    // start queue stream
    media_tracker.queue.clear();
    media_tracker.queue.begin();
    is_streams_opened = true;
  }

  int sbc_buffer_length_sbc() { return get_encoder().frameLengthEncoded(); }

  int sbc_buffer_length_pcm() { return get_encoder().frameLengthDecoded(); }

  void a2dp_arduino_send_media_packet(void) {
    TRACED();

    // determine data
    int num_bytes_in_frame = sbc_buffer_length_sbc();
    int available = media_tracker.queue.available();
    int num_frames = available / num_bytes_in_frame;

    // log output
    LOGI("a2dp_arduino_send_media_packet: %d packets (%d bytes)", num_frames,
         available);
    if (available % num_bytes_in_frame) {
      LOGW("Invalid number of available bytes: available: %d, frame_size: %d",
           available, num_bytes_in_frame);
    }

    // send out data
    uint8_t buffer[available + 1];
    // Prepend SBC Header // (fragmentation << 7) | (starting_packet << 6) |
    // (last_packet << 5) | num_frames;
    media_tracker.queue.readBytes(buffer + 1, available);
    buffer[0] = num_frames;
    int rc = avdtp_source_stream_send_media_payload_rtp(
        media_tracker.a2dp_cid, media_tracker.local_seid, 0, 0, buffer,
        available + 1);

    if (rc != ERROR_CODE_SUCCESS) {
      LOGE("avdtp_source_stream_send_media_payload_rtp: %d", rc);
    }

    // allow to process the next packets
    media_tracker.sbc_is_busy = false;
  }

  int a2dp_arduino_fill_sbc_audio_buffer(
      a2dp_media_sending_context_t *context) {
    TRACED();
    int len = sbc_buffer_length_pcm() * SBC_PACKET_COUNT;
    uint8_t pcm_buffer[len];
    while (media_tracker.queue.available() == 0) {
      size_t bytes = volume_stream.readBytes(pcm_buffer, len);
      LOGD("readBytes: %d -> %d", len, bytes);

      size_t bytes_written = encoder_stream.write(pcm_buffer, bytes);
      LOGD("write: %d -> %d", bytes_written, media_tracker.queue.available());
    }
    int available = media_tracker.queue.available();
    LOGD("sbc bytes: %d", available);
    return available;
  }

  void a2dp_audio_timeout_handler(btstack_timer_source_t *timer) {
    TRACED();
    a2dp_media_sending_context_t *context =
        (a2dp_media_sending_context_t *)btstack_run_loop_get_timer_context(
            timer);
    btstack_run_loop_set_timer(&context->audio_timer, AUDIO_TIMEOUT_MS);
    btstack_run_loop_add_timer(&context->audio_timer);
    uint32_t now = btstack_run_loop_get_time_ms();

    if (context->sbc_is_busy) return;
    if (!context->is_streaming) return;

    a2dp_arduino_fill_sbc_audio_buffer(context);

    // schedule sending
    context->sbc_is_busy = true;
    a2dp_source_stream_endpoint_request_can_send_now(context->a2dp_cid,
                                                     context->local_seid);
  }

  void a2dp_arduino_timer_start(a2dp_media_sending_context_t *context) {
    TRACED();
    context->max_media_payload_size = btstack_min(
        a2dp_max_media_payload_size(context->a2dp_cid, context->local_seid),
        SBC_STORAGE_SIZE);
    context->sbc_is_busy = false;
    context->is_streaming = true;
    btstack_run_loop_remove_timer(&context->audio_timer);
    btstack_run_loop_set_timer_handler(&context->audio_timer,
                                       source_a2dp_audio_timeout_handler);
    btstack_run_loop_set_timer_context(&context->audio_timer, context);
    btstack_run_loop_set_timer(&context->audio_timer, AUDIO_TIMEOUT_MS);
    btstack_run_loop_add_timer(&context->audio_timer);
  }

  void a2dp_arduino_timer_stop(a2dp_media_sending_context_t *context) {
    TRACED();
    // context->time_audio_data_sent = 0;
    // context->acc_num_missed_samples = 0;
    // context->samples_ready = 0;
    context->is_streaming = true;
    context->sbc_is_busy = false;
    btstack_run_loop_remove_timer(&context->audio_timer);
  }

  void a2dp_source_arduino_start_scanning(void) {
    TRACED();
    LOGI("Start scanning...");
    gap_inquiry_start(A2DP_SOURCE_arduino_INQUIRY_DURATION_1280MS);
    scan_active = true;
  }

  virtual void hci_packet_handler(uint8_t packet_type, uint16_t channel,
                                        uint8_t *packet, uint16_t size) {
    TRACED();
    UNUSED(channel);
    UNUSED(size);
    if (packet_type != HCI_EVENT_PACKET) return;
    uint8_t status;
    UNUSED(status);

    bd_addr_t address;
    uint32_t cod;
    bool name_matches = remote_name == nullptr;

    // Service Class: Rendering | Audio, Major Device Class: Audio
    const uint32_t bluetooth_speaker_cod = 0x200000 | 0x040000 | 0x000400;

    switch (hci_event_packet_get_type(packet)) {
      case BTSTACK_EVENT_STATE:
        if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) return;
        a2dp_source_arduino_start_scanning();
        break;
      case HCI_EVENT_PIN_CODE_REQUEST:
        LOGI("Pin code request - using '0000'");
        hci_event_pin_code_request_get_bd_addr(packet, address);
        gap_pin_code_response(address, "0000");
        break;
      case GAP_EVENT_INQUIRY_RESULT:
        gap_event_inquiry_result_get_bd_addr(packet, address);
        // print info
        LOGI("Device found: %s ", bd_addr_to_str(address));
        cod = gap_event_inquiry_result_get_class_of_device(packet);
        LOGI("- COD: %06" PRIx32, cod);
        if (gap_event_inquiry_result_get_rssi_available(packet)) {
          LOGI("- rssi %d dBm",
               (int8_t)gap_event_inquiry_result_get_rssi(packet));
        }
        if (gap_event_inquiry_result_get_name_available(packet)) {
          char name_buffer[240];
          int name_len = gap_event_inquiry_result_get_name_len(packet);
          memcpy(name_buffer, gap_event_inquiry_result_get_name(packet),
                 name_len);
          name_buffer[name_len] = 0;
          LOGI("- name '%s'", name_buffer);
          // check if the name is the requested remote name
          if (remote_name != nullptr) {
            name_matches = Str(name_buffer).equalsIgnoreCase(remote_name);
          }
        }
        if (name_matches &&
            (cod & bluetooth_speaker_cod) == bluetooth_speaker_cod) {
          memcpy(device_addr, address, 6);
          LOGI("Bluetooth speaker detected, trying to connect to %s...",
               bd_addr_to_str(device_addr));
          scan_active = false;
          gap_inquiry_stop();
          a2dp_source_establish_stream(device_addr, &media_tracker.a2dp_cid);
        }
        break;
      case GAP_EVENT_INQUIRY_COMPLETE:
        if (scan_active) {
          LOGW("No Bluetooth speakers found, scanning again...");
          gap_inquiry_start(A2DP_SOURCE_arduino_INQUIRY_DURATION_1280MS);
        }
        break;
      default:
        break;
    }
  }

  virtual void a2dp_packet_handler(uint8_t packet_type, uint16_t channel,
                                         uint8_t *packet, uint16_t size) {
    TRACED();
    UNUSED(channel);
    UNUSED(size);
    uint8_t status;
    uint8_t local_seid;
    bd_addr_t address;
    uint16_t cid;

    avdtp_channel_mode_t channel_mode;
    uint8_t allocation_method;

    if (packet_type != HCI_EVENT_PACKET) return;
    if (hci_event_packet_get_type(packet) != HCI_EVENT_A2DP_META) return;

    switch (hci_event_a2dp_meta_get_subevent_code(packet)) {
      case A2DP_SUBEVENT_SIGNALING_CONNECTION_ESTABLISHED:
        a2dp_subevent_signaling_connection_established_get_bd_addr(packet,
                                                                   address);
        cid =
            a2dp_subevent_signaling_connection_established_get_a2dp_cid(packet);
        status =
            a2dp_subevent_signaling_connection_established_get_status(packet);

        if (status != ERROR_CODE_SUCCESS) {
          LOGE(
              "A2DP Source: Connection failed, status 0x%02x, cid 0x%02x", 
              status, cid);
          break;
        }
        LOGI("A2DP Source: Connected to address %s", bd_addr_to_str(address));
        break;

      case A2DP_SUBEVENT_SIGNALING_MEDIA_CODEC_SBC_CONFIGURATION: {
        cid =
            avdtp_subevent_signaling_media_codec_sbc_configuration_get_avdtp_cid(
                packet);
        if (cid != media_tracker.a2dp_cid) return;

        media_tracker.remote_seid =
            a2dp_subevent_signaling_media_codec_sbc_configuration_get_remote_seid(
                packet);

        A2DPEncoder &enc = get_encoder();
        enc.setValues(cid, packet, size);
        // Setup SBC decoder
        auto info = enc.audioInfo();
        source_a2dp_configure_sample_rate(info.sample_rate);
        open_audio_streams();
        break;
      }

      case A2DP_SUBEVENT_SIGNALING_DELAY_REPORTING_CAPABILITY:
        LOGI(
            "A2DP Source: remote supports delay report, remote seid %d",
            avdtp_subevent_signaling_delay_reporting_capability_get_remote_seid(
                packet));
        break;
      case A2DP_SUBEVENT_SIGNALING_CAPABILITIES_DONE:
        LOGI(
            "A2DP Source: All capabilities reported, remote seid %d",
            avdtp_subevent_signaling_capabilities_done_get_remote_seid(packet));
        break;

      case A2DP_SUBEVENT_SIGNALING_DELAY_REPORT:
        LOGI(
            "A2DP Source: Received delay report of %d.%0d ms, local seid "
            "%d",
            avdtp_subevent_signaling_delay_report_get_delay_100us(packet) / 10,
            avdtp_subevent_signaling_delay_report_get_delay_100us(packet) % 10,
            avdtp_subevent_signaling_delay_report_get_local_seid(packet));
        break;

      case A2DP_SUBEVENT_STREAM_ESTABLISHED:
        a2dp_subevent_stream_established_get_bd_addr(packet, address);
        status = a2dp_subevent_stream_established_get_status(packet);
        if (status != ERROR_CODE_SUCCESS) {
          LOGE("A2DP Source: Stream failed, status 0x%02x.", status);
          break;
        }

        local_seid = a2dp_subevent_stream_established_get_local_seid(packet);
        cid = a2dp_subevent_stream_established_get_a2dp_cid(packet);

        LOGI(
            "A2DP Source: Stream established a2dp_cid 0x%02x, local_seid "
            "0x%02x, remote_seid 0x%02x",
            cid, local_seid,
            a2dp_subevent_stream_established_get_remote_seid(packet));

        source_a2dp_configure_sample_rate(current_sample_rate);
        media_tracker.stream_opened = 1;
        status = a2dp_source_start_stream(media_tracker.a2dp_cid,
                                          media_tracker.local_seid);
        break;

      case A2DP_SUBEVENT_STREAM_RECONFIGURED:
        status = a2dp_subevent_stream_reconfigured_get_status(packet);
        local_seid = a2dp_subevent_stream_reconfigured_get_local_seid(packet);
        cid = a2dp_subevent_stream_reconfigured_get_a2dp_cid(packet);

        if (status != ERROR_CODE_SUCCESS) {
          LOGE(
              "A2DP Source: Stream reconfiguration failed with status "
              "0x%02x",
              status);
          break;
        }

        LOGI(
            "A2DP Source: Stream reconfigured a2dp_cid 0x%02x, local_seid "
            "0x%02x",
            cid, local_seid);
        source_a2dp_configure_sample_rate(new_sample_rate);
        status = a2dp_source_start_stream(media_tracker.a2dp_cid,
                                          media_tracker.local_seid);
        break;

      case A2DP_SUBEVENT_STREAM_STARTED:
        local_seid = a2dp_subevent_stream_started_get_local_seid(packet);
        cid = a2dp_subevent_stream_started_get_a2dp_cid(packet);

        play_info.status = AVRCP_PLAYBACK_STATUS_PLAYING;
        if (media_tracker.avrcp_cid) {
          avrcp_target_set_now_playing_info(media_tracker.avrcp_cid,
                                            &track_info, track_count);
          avrcp_target_set_playback_status(media_tracker.avrcp_cid,
                                           AVRCP_PLAYBACK_STATUS_PLAYING);
        }
        a2dp_arduino_timer_start(&media_tracker);
        LOGI(
            "A2DP Source: Stream started, a2dp_cid 0x%02x, local_seid "
            "0x%02x",
            cid, local_seid);
        break;

      case A2DP_SUBEVENT_STREAMING_CAN_SEND_MEDIA_PACKET_NOW:
        local_seid =
            a2dp_subevent_streaming_can_send_media_packet_now_get_local_seid(
                packet);
        cid =
            a2dp_subevent_signaling_media_codec_sbc_configuration_get_a2dp_cid(
                packet);
        a2dp_arduino_send_media_packet();
        break;

      case A2DP_SUBEVENT_STREAM_SUSPENDED:
        local_seid = a2dp_subevent_stream_suspended_get_local_seid(packet);
        cid = a2dp_subevent_stream_suspended_get_a2dp_cid(packet);

        play_info.status = AVRCP_PLAYBACK_STATUS_PAUSED;
        if (media_tracker.avrcp_cid) {
          avrcp_target_set_playback_status(media_tracker.avrcp_cid,
                                           AVRCP_PLAYBACK_STATUS_PAUSED);
        }
        LOGI(
            "A2DP Source: Stream paused, a2dp_cid 0x%02x, local_seid "
            "0x%02x",
            cid, local_seid);

        a2dp_arduino_timer_stop(&media_tracker);
        break;

      case A2DP_SUBEVENT_STREAM_RELEASED:
        play_info.status = AVRCP_PLAYBACK_STATUS_STOPPED;
        cid = a2dp_subevent_stream_released_get_a2dp_cid(packet);
        local_seid = a2dp_subevent_stream_released_get_local_seid(packet);

        LOGI(
            "A2DP Source: Stream released, a2dp_cid 0x%02x, local_seid "
            "0x%02x",
            cid, local_seid);

        if (cid == media_tracker.a2dp_cid) {
          media_tracker.stream_opened = 0;
          LOGI("A2DP Source: Stream released.");
        }
        if (media_tracker.avrcp_cid) {
          avrcp_target_set_now_playing_info(media_tracker.avrcp_cid, NULL,
                                            track_count);
          avrcp_target_set_playback_status(media_tracker.avrcp_cid,
                                           AVRCP_PLAYBACK_STATUS_STOPPED);
        }
        a2dp_arduino_timer_stop(&media_tracker);
        break;
      case A2DP_SUBEVENT_SIGNALING_CONNECTION_RELEASED:
        cid = a2dp_subevent_signaling_connection_released_get_a2dp_cid(packet);
        if (cid == media_tracker.a2dp_cid) {
          media_tracker.avrcp_cid = 0;
          media_tracker.a2dp_cid = 0;
          LOGI("A2DP Source: Signaling released.");
        }
        break;
      default:
        break;
    }
  }

  void avrcp_packet_handler(uint8_t packet_type, uint16_t channel,
                                  uint8_t *packet, uint16_t size) {
    TRACED();
    UNUSED(channel);
    UNUSED(size);
    bd_addr_t event_addr;
    uint16_t local_cid;
    uint8_t status = ERROR_CODE_SUCCESS;

    if (packet_type != HCI_EVENT_PACKET) return;
    if (hci_event_packet_get_type(packet) != HCI_EVENT_AVRCP_META) return;

    switch (packet[2]) {
      case AVRCP_SUBEVENT_CONNECTION_ESTABLISHED:
        local_cid = avrcp_subevent_connection_established_get_avrcp_cid(packet);
        status = avrcp_subevent_connection_established_get_status(packet);
        if (status != ERROR_CODE_SUCCESS) {
          LOGE("AVRCP: Connection failed, local cid 0x%02x, status 0x%02x",
               local_cid, status);
          return;
        }
        media_tracker.avrcp_cid = local_cid;
        avrcp_subevent_connection_established_get_bd_addr(packet, event_addr);

        LOGI("AVRCP: Channel to %s successfully opened, avrcp_cid 0x%02x",
             bd_addr_to_str(event_addr), media_tracker.avrcp_cid);

        avrcp_target_support_event(
            media_tracker.avrcp_cid,
            AVRCP_NOTIFICATION_EVENT_PLAYBACK_STATUS_CHANGED);
        avrcp_target_support_event(media_tracker.avrcp_cid,
                                   AVRCP_NOTIFICATION_EVENT_TRACK_CHANGED);
        avrcp_target_support_event(
            media_tracker.avrcp_cid,
            AVRCP_NOTIFICATION_EVENT_NOW_PLAYING_CONTENT_CHANGED);
        avrcp_target_set_now_playing_info(media_tracker.avrcp_cid, NULL,
                                          track_count);

        LOGI("Enable Volume Change notification");
        avrcp_controller_enable_notification(
            media_tracker.avrcp_cid, AVRCP_NOTIFICATION_EVENT_VOLUME_CHANGED);
        LOGI("Enable Battery Status Change notification");
        avrcp_controller_enable_notification(
            media_tracker.avrcp_cid,
            AVRCP_NOTIFICATION_EVENT_BATT_STATUS_CHANGED);
        return;

      case AVRCP_SUBEVENT_CONNECTION_RELEASED:
        LOGI("AVRCP Target: Disconnected, avrcp_cid 0x%02x",
             avrcp_subevent_connection_released_get_avrcp_cid(packet));
        media_tracker.avrcp_cid = 0;
        return;
      default:
        break;
    }

    if (status != ERROR_CODE_SUCCESS) {
      LOGE("Responding to event 0x%02x failed with status 0x%02x", packet[2],
           status);
    }
  }

  virtual void avrcp_controller_packet_handler(uint8_t packet_type,
                                               uint16_t channel,
                                               uint8_t *packet, uint16_t size) {
    UNUSED(channel);
    UNUSED(size);

    if (packet_type != HCI_EVENT_PACKET) return;
    if (hci_event_packet_get_type(packet) != HCI_EVENT_AVRCP_META) return;
    if (!media_tracker.avrcp_cid) return;

    switch (packet[2]) {
      case AVRCP_SUBEVENT_NOTIFICATION_VOLUME_CHANGED:
        printf("AVRCP Controller: Notification Absolute Volume %d %%\n",
               avrcp_subevent_notification_volume_changed_get_absolute_volume(
                   packet) *
                   100 / 127);
        break;
      case AVRCP_SUBEVENT_NOTIFICATION_EVENT_BATT_STATUS_CHANGED:
        // see avrcp_battery_status_t
        printf(
            "AVRCP Controller: Notification Battery Status 0x%02x\n",
            avrcp_subevent_notification_event_batt_status_changed_get_battery_status(
                packet));
        break;
      case AVRCP_SUBEVENT_NOTIFICATION_STATE:
        printf("AVRCP Controller: Notification %s - %s\n",
               avrcp_event2str(
                   avrcp_subevent_notification_state_get_event_id(packet)),
               avrcp_subevent_notification_state_get_enabled(packet) != 0
                   ? "enabled"
                   : "disabled");
        break;
      default:
        break;
    }
  }

  virtual void avrcp_target_packet_handler(uint8_t packet_type,
                                                 uint16_t channel,
                                                 uint8_t *packet,
                                                 uint16_t size) {
    TRACED();
    UNUSED(channel);
    UNUSED(size);
    uint8_t status = ERROR_CODE_SUCCESS;

    if (packet_type != HCI_EVENT_PACKET) return;
    if (hci_event_packet_get_type(packet) != HCI_EVENT_AVRCP_META) return;

    bool button_pressed;
    char const *button_state;
    avrcp_operation_id_t operation_id;

    switch (packet[2]) {
      case AVRCP_SUBEVENT_PLAY_STATUS_QUERY:
        status = avrcp_target_play_status(
            media_tracker.avrcp_cid, play_info.song_length_ms,
            play_info.song_position_ms, play_info.status);
        break;
      // case AVRCP_SUBEVENT_NOW_PLAYING_INFO_QUERY:
      //     status = avrcp_target_now_playing_info(avrcp_cid);
      //     break;
      case AVRCP_SUBEVENT_OPERATION:
        operation_id =
            (avrcp_operation_id_t)avrcp_subevent_operation_get_operation_id(
                (const uint8_t *)packet);
        button_pressed =
            avrcp_subevent_operation_get_button_pressed(packet) > 0;
        button_state = button_pressed ? "PRESS" : "RELEASE";

        LOGI("AVRCP Target: operation %s (%s)",
             avrcp_operation2str(operation_id), button_state);

        if (!button_pressed) {
          break;
        }
        switch (operation_id) {
          case AVRCP_OPERATION_ID_PLAY:
            status = a2dp_source_start_stream(media_tracker.a2dp_cid,
                                              media_tracker.local_seid);
            break;
          case AVRCP_OPERATION_ID_PAUSE:
            status = a2dp_source_pause_stream(media_tracker.a2dp_cid,
                                              media_tracker.local_seid);
            break;
          case AVRCP_OPERATION_ID_STOP:
            status = a2dp_source_disconnect(media_tracker.a2dp_cid);
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }

    if (status != ERROR_CODE_SUCCESS) {
      LOGE("Responding to event 0x%02x failed with status 0x%02x", packet[2],
           status);
    }
  }

  void setupTrack() {
    static bool is_track_setup = false;
    if (!is_track_setup) {
      TRACED();
      char *empty = (char *)"n/a";
      const char id[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
      memcpy(track_info.track_id, id, 8);
      track_info.track_nr = 1;
      track_info.title = empty;
      track_info.artist = empty;
      track_info.album = empty;
      track_info.genre = empty;
      track_info.song_length_ms = 0xFFFFFFFF;
      track_info.song_position_ms = 0;
      is_track_setup = true;
    }
  }

} A2DPSource;

// -- Implement Callback functions which forward calls to A2DPSource

void source_a2dp_packet_handler(uint8_t packet_type, uint16_t channel,
                                uint8_t *event, uint16_t event_size) {
  A2DPSource.a2dp_packet_handler(packet_type, channel, event, event_size);
}

void source_avrcp_target_packet_handler(uint8_t packet_type, uint16_t channel,
                                        uint8_t *packet, uint16_t size) {
  A2DPSource.avrcp_target_packet_handler(packet_type, channel, packet,
                                               size);
}

void source_hci_packet_handler(uint8_t packet_type, uint16_t channel,
                               uint8_t *packet, uint16_t size) {
  A2DPSource.hci_packet_handler(packet_type, channel, packet, size);
}

void source_avrcp_packet_handler(uint8_t packet_type, uint16_t channel,
                                 uint8_t *packet, uint16_t size) {
  A2DPSource.avrcp_packet_handler(packet_type, channel, packet, size);
}

void source_a2dp_audio_timeout_handler(btstack_timer_source_t *timer) {
  A2DPSource.a2dp_audio_timeout_handler(timer);
}

void source_avrcp_controller_packet_handler(uint8_t packet_type,
                                            uint16_t channel, uint8_t *packet,
                                            uint16_t size) {
  A2DPSource.avrcp_controller_packet_handler(packet_type, channel, packet,
                                                   size);
}

}  // namespace btstack_a2dp
