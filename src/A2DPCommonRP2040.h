#pragma once
#include "A2DPConfigRP2040.h"
#include "bluetooth.h"
#include "btstack.h"
#include "btstack_defines.h"
#include <pico/cyw43_arch.h>

#include "AudioTools.h"
#include "AudioCodecs/CodecSBC.h"

//#define BYTES_PER_FRAME     (2*NUM_CHANNELS)
//#define BYTES_PER_AUDIO_SAMPLE (2 * NUM_CHANNELS)


namespace a2dp_rp2040 {

enum MetadataType {
  MDTitle,
  MDArtist,
  MDAlbum,
  MDGenre,
  MDPlaybackPosMs,
  MDTrack,
  MDTracks,
  MDSongLen,
  MDSongPos
};

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
  /// Sets the volume
  bool setVolume(uint8_t volPercent) {
    TRACEI();
    volume_percentage = volPercent;
    if (volume_percentage > 100) {
      volume_percentage = 100;
    }
    if (volume_percentage < 0) {
      volume_percentage = 0;
    }
    int volume = volume_percentage * 127 / 100;
    int status = avrcp_target_volume_changed(get_avrcp_cid(), volume);
    avrcp_volume_changed(volume);
    return status == 0;
  }

  /// Provides the actual volume (as %) in the range from 0 to 100
  uint8_t volume() { return volume_percentage; }

  /// Defines the callback method to receive metadata events
  void setMetadataCallback(void (*callback)(MetadataType type, const char *data,
                                            uint32_t value)) {
    metadata_callback = callback;
  }

  void lockBluetooth() {
    async_context_acquire_lock_blocking(cyw43_arch_async_context());
  }

  void unlockBluetooth() {
    async_context_release_lock(cyw43_arch_async_context());
  }

  /// avrcp play
  bool play() {
    TRACEI();
    return 0 == avrcp_controller_play(get_avrcp_cid());
  }

  /// avrcp stop
  bool stop() {
    TRACEI();
    return 0 == avrcp_controller_stop(get_avrcp_cid());
  }

  /// avrcp pause
  bool pause() {
    TRACEI();
    return 0 == avrcp_controller_pause(get_avrcp_cid());
  }

  /// avrcp forward
  bool next() {
    TRACEI();
    return 0 == avrcp_controller_forward(get_avrcp_cid());
  }

  /// avrcp backward
  bool previous() {
    TRACEI();
    return 0 == avrcp_controller_backward(get_avrcp_cid());
  }

  /// avrcp fast_forwar
  bool fastForward(bool start) {
    TRACEI();
    if (start) {
      return 0 == avrcp_controller_press_and_hold_fast_forward(get_avrcp_cid());
    } else {
      return 0 == avrcp_controller_release_press_and_hold_cmd(get_avrcp_cid());
    }
  }

  /// avrcp rewind
  bool rewind(bool start) {
    TRACEI();
    if (start) {
      return 0 == avrcp_controller_press_and_hold_rewind(get_avrcp_cid());

    } else {
      return 0 == avrcp_controller_release_press_and_hold_cmd(get_avrcp_cid());
    }
  }

  bool isPlaying() { return is_playing; }

protected:
  VolumeStream volume_stream;
  int volume_percentage = 100;
  bool is_playing = false;
  void (*metadata_callback)(MetadataType type, const char *data,
                            uint32_t value) = nullptr;

  virtual int get_avrcp_cid() = 0;

  virtual void set_playing(bool playing) { is_playing = playing; }

  virtual void local_avrcp_controller_packet_handler(uint8_t packet_type,
                                                     uint16_t channel,
                                                     uint8_t *packet,
                                                     uint16_t size) {
    TRACED();
    UNUSED(channel);
    UNUSED(size);

    // helper to print c strings
    uint8_t avrcp_subevent_value[256];
    uint8_t play_status;

    // a2dp_sink_arduino_avrcp_connection_t *avrcp_connection =
    //     &a2dp_sink_arduino_avrcp_connection;

    if (packet_type != HCI_EVENT_PACKET)
      return;
    if (hci_event_packet_get_type(packet) != HCI_EVENT_AVRCP_META)
      return;
    if (get_avrcp_cid() == 0)
      return;

    memset(avrcp_subevent_value, 0, sizeof(avrcp_subevent_value));
    switch (packet[2]) {

    case AVRCP_SUBEVENT_NOTIFICATION_VOLUME_CHANGED: {
      int vol = avrcp_subevent_notification_volume_changed_get_absolute_volume(
                    packet) * 100 / 127;
      LOGI("AVRCP Controller: Notification Absolute Volume %d %%", vol);
      avrcp_volume_changed(vol);
    } break;
    case AVRCP_SUBEVENT_NOTIFICATION_EVENT_BATT_STATUS_CHANGED:
      // see avrcp_battery_status_t
      LOGI(
          "AVRCP Controller: Notification Battery Status %d",
          avrcp_subevent_notification_event_batt_status_changed_get_battery_status(
              packet));
      break;
    case AVRCP_SUBEVENT_NOTIFICATION_STATE:
      LOGI("AVRCP Controller: Notification %s - %s",
           avrcp_event2str(
               avrcp_subevent_notification_state_get_event_id(packet)),
           avrcp_subevent_notification_state_get_enabled(packet) != 0
               ? "enabled"
               : "disabled");
      break;

    case AVRCP_SUBEVENT_NOTIFICATION_PLAYBACK_POS_CHANGED: {
      uint32_t pos_ms =
          avrcp_subevent_notification_playback_pos_changed_get_playback_position_ms(
              packet);
      LOGI("AVRCP Controller: Playback position changed, position %d ms",
           (unsigned int)pos_ms);
      if (metadata_callback)
        metadata_callback(MDPlaybackPosMs, nullptr, pos_ms);
    } break;
    case AVRCP_SUBEVENT_NOTIFICATION_PLAYBACK_STATUS_CHANGED:
      LOGI(
          "AVRCP Controller: Playback status changed %s\n",
          avrcp_play_status2str(
              avrcp_subevent_notification_playback_status_changed_get_play_status(
                  packet)));
      play_status =
          avrcp_subevent_notification_playback_status_changed_get_play_status(
              packet);
      switch (play_status) {
      case AVRCP_PLAYBACK_STATUS_PLAYING:
        // avrcp_connection->playing = true;
        set_playing(true);
        break;
      default:
        // avrcp_connection->playing = false;
        set_playing(false);
        break;
      }
      LOGI("AVRCP Controller: Playback status changed %s",
           avrcp_play_status2str(play_status));
      return;
    case AVRCP_SUBEVENT_NOTIFICATION_NOW_PLAYING_CONTENT_CHANGED:
      LOGI("AVRCP Controller: Playing content changed");
      return;
    case AVRCP_SUBEVENT_NOTIFICATION_TRACK_CHANGED:
      LOGI("AVRCP Controller: Track changed");
      return;
    case AVRCP_SUBEVENT_NOTIFICATION_AVAILABLE_PLAYERS_CHANGED:
      LOGI("AVRCP Controller: Changed");
      return;
    case AVRCP_SUBEVENT_SHUFFLE_AND_REPEAT_MODE: {
      uint8_t shuffle_mode =
          avrcp_subevent_shuffle_and_repeat_mode_get_shuffle_mode(packet);
      uint8_t repeat_mode =
          avrcp_subevent_shuffle_and_repeat_mode_get_repeat_mode(packet);
      LOGI("AVRCP Controller: %s, %s", avrcp_shuffle2str(shuffle_mode),
           avrcp_repeat2str(repeat_mode));
      break;
    }
    case AVRCP_SUBEVENT_NOW_PLAYING_TRACK_INFO: {
      uint32_t val = avrcp_subevent_now_playing_track_info_get_track(packet);
      LOGI("AVRCP Controller:     Track: %d", val);
      if (metadata_callback)
        metadata_callback(MDTrack, nullptr, val);
    } break;

    case AVRCP_SUBEVENT_NOW_PLAYING_TOTAL_TRACKS_INFO: {
      uint32_t val =
          avrcp_subevent_now_playing_total_tracks_info_get_total_tracks(packet);
      LOGI("AVRCP Controller:     Total Tracks: %d", val);
      if (metadata_callback)
        metadata_callback(MDTracks, nullptr, val);
    } break;

    case AVRCP_SUBEVENT_NOW_PLAYING_TITLE_INFO:
      if (avrcp_subevent_now_playing_title_info_get_value_len(packet) > 0) {
        memcpy(avrcp_subevent_value,
               avrcp_subevent_now_playing_title_info_get_value(packet),
               avrcp_subevent_now_playing_title_info_get_value_len(packet));
        LOGI("AVRCP Controller:     Title: %s", avrcp_subevent_value);
        if (metadata_callback)
          metadata_callback(MDTitle, (const char *)avrcp_subevent_value, 0);
      }
      break;

    case AVRCP_SUBEVENT_NOW_PLAYING_ARTIST_INFO:
      if (avrcp_subevent_now_playing_artist_info_get_value_len(packet) > 0) {
        memcpy(avrcp_subevent_value,
               avrcp_subevent_now_playing_artist_info_get_value(packet),
               avrcp_subevent_now_playing_artist_info_get_value_len(packet));
        LOGI("AVRCP Controller:     Artist: %s", avrcp_subevent_value);
        if (metadata_callback)
          metadata_callback(MDArtist, (const char *)avrcp_subevent_value, 0);
      }
      break;

    case AVRCP_SUBEVENT_NOW_PLAYING_ALBUM_INFO:
      if (avrcp_subevent_now_playing_album_info_get_value_len(packet) > 0) {
        memcpy(avrcp_subevent_value,
               avrcp_subevent_now_playing_album_info_get_value(packet),
               avrcp_subevent_now_playing_album_info_get_value_len(packet));
        LOGI("AVRCP Controller:     Album: %s", avrcp_subevent_value);
        if (metadata_callback)
          metadata_callback(MDAlbum, (const char *)avrcp_subevent_value, 0);
      }
      break;

    case AVRCP_SUBEVENT_NOW_PLAYING_GENRE_INFO:
      if (avrcp_subevent_now_playing_genre_info_get_value_len(packet) > 0) {
        memcpy(avrcp_subevent_value,
               avrcp_subevent_now_playing_genre_info_get_value(packet),
               avrcp_subevent_now_playing_genre_info_get_value_len(packet));
        LOGI("AVRCP Controller:     Genre: %s", avrcp_subevent_value);
        if (metadata_callback)
          metadata_callback(MDGenre, (const char *)avrcp_subevent_value, 0);
      }
      break;

    case AVRCP_SUBEVENT_PLAY_STATUS: {
      uint32_t len = avrcp_subevent_play_status_get_song_length(packet);
      uint32_t pos = avrcp_subevent_play_status_get_song_position(packet);
      LOGI("AVRCP Controller: Song length %" PRIu32
           " ms, Song position %" PRIu32 " ms, Play status %s",
           len, pos,
           avrcp_play_status2str(
               avrcp_subevent_play_status_get_play_status(packet)));

      if (metadata_callback) {
        metadata_callback(MDSongLen, nullptr, len);
        metadata_callback(MDSongPos, nullptr, pos);
      }
    } break;

    case AVRCP_SUBEVENT_OPERATION_COMPLETE:
      LOGI("AVRCP Controller: %s complete",
           avrcp_operation2str(
               avrcp_subevent_operation_complete_get_operation_id(packet)));
      break;

    case AVRCP_SUBEVENT_OPERATION_START:
      LOGI("AVRCP Controller: %s start",
           avrcp_operation2str(
               avrcp_subevent_operation_start_get_operation_id(packet)));
      break;

    case AVRCP_SUBEVENT_NOTIFICATION_EVENT_TRACK_REACHED_END:
      LOGI("AVRCP Controller: Track reached end");
      break;

    case AVRCP_SUBEVENT_PLAYER_APPLICATION_VALUE_RESPONSE:
      LOGI(
          "AVRCP Controller: Set Player App Value %s",
          avrcp_ctype2str(
              avrcp_subevent_player_application_value_response_get_command_type(
                  packet)));
      break;

    default:
      break;
    }
  }

  virtual int16_t get_max_input_amplitude() = 0;

  void avrcp_volume_changed(uint8_t volume) {
    // calculate eff volume factor
    float max_current = get_max_input_amplitude();
    float max_factor = 32767.0f / max_current;
    float vol_float = 0.01f * volume_percentage;
    // We adjust the volume: if we receive only max amplitude of 2500 we can
    // multiply the values by a factor of 13
    float factor = mapFloat(vol_float, 0.0, 1.0, 0.0, max_factor);
    LOGI("avrcp_volume_changed: %d -> %f", volume, factor);
    // adjust volume
    volume_stream.setVolume(factor);
  }
};

} // namespace a2dp_rp2040
