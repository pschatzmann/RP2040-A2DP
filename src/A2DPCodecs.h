#pragma once
#include "AudioCodecs/CodecSBC.h"
#include "AudioTools.h"

namespace btstack_a2dp {

/**
 * @brief sbc attributes
 */
struct media_codec_configuration_sbc_t {
  uint8_t reconfigure;
  uint8_t num_channels;
  uint16_t sampling_frequency;
  uint8_t block_length;
  uint8_t subbands;
  uint8_t min_bitpool_value;
  uint8_t max_bitpool_value;
  btstack_sbc_channel_mode_t channel_mode;
  btstack_sbc_allocation_method_t allocation_method;

  void dump() {
    TRACED();
    LOGI("- num_channels: %d", num_channels);
    LOGI("- sampling_frequency: %d", sampling_frequency);
    LOGI("- channel_mode: %d", channel_mode);
    LOGI("- block_length: %d", block_length);
    LOGI("- subbands: %d", subbands);
    LOGI("- allocation_method: %d", allocation_method);
    LOGI("- bitpool_value [%d, %d]", min_bitpool_value, max_bitpool_value);
  }
};

/**
 * @brief A2DPEncoder: Common Encoder functionality that is needed by A2DP
 * @author Phil Schatzmann
 */
class A2DPEncoder {
public:
  virtual uint8_t *config() = 0;
  virtual int configSize() = 0;
  virtual void setValues(uint16_t cid, uint8_t *packet, uint16_t size) = 0;
  virtual void begin() = 0;
  virtual uint8_t *codecCapabilities() = 0;
  virtual int codecCapabilitiesSize() = 0;
  virtual AudioEncoder &encoder() = 0;
  virtual void dump() = 0;
  virtual int frameLengthEncoded() = 0;
  virtual int frameLengthDecoded() = 0;
  virtual AudioBaseInfo audioInfo() = 0;
  virtual avdtp_media_codec_type_t codecType() = 0;
};

/**
 * @brief A2DPEncoder: Common Decoder functionality that is needed by A2DP
 * @author Phil Schatzmann
 */
class A2DPDecoder {
public:
  virtual uint8_t *config() = 0;
  virtual int configSize() = 0;
  virtual void begin() = 0;
  virtual uint8_t *codecCapabilities() = 0;
  virtual int codecCapabilitiesSize() = 0;
  virtual AudioDecoder &decoder() = 0;
};

/**
 * @brief SBC Encoder implementation
 * @author Phil Schatzmann
 */
class A2DPEncoderSBC : public A2DPEncoder {
public:
  A2DPEncoderSBC(){
    TRACEI();
  }
  uint8_t *config() override { return media_sbc_codec_configuration; }

  int configSize() override { return sizeof(media_sbc_codec_configuration); }

  void setValues(uint16_t cid, uint8_t *packet, uint16_t size) override {
    avdtp_channel_mode_t channel_mode;
    uint8_t allocation_method;

    sbc_config.reconfigure =
        a2dp_subevent_signaling_media_codec_sbc_configuration_get_reconfigure(
            packet);
    sbc_config.num_channels =
        a2dp_subevent_signaling_media_codec_sbc_configuration_get_num_channels(
            packet);
    sbc_config.sampling_frequency =
        a2dp_subevent_signaling_media_codec_sbc_configuration_get_sampling_frequency(
            packet);
    sbc_config.block_length =
        a2dp_subevent_signaling_media_codec_sbc_configuration_get_block_length(
            packet);
    sbc_config.subbands =
        a2dp_subevent_signaling_media_codec_sbc_configuration_get_subbands(
            packet);
    sbc_config.min_bitpool_value =
        a2dp_subevent_signaling_media_codec_sbc_configuration_get_min_bitpool_value(
            packet);
    sbc_config.max_bitpool_value =
        a2dp_subevent_signaling_media_codec_sbc_configuration_get_max_bitpool_value(
            packet);

    channel_mode = (avdtp_channel_mode_t)
        a2dp_subevent_signaling_media_codec_sbc_configuration_get_channel_mode(
            packet);
    allocation_method =
        a2dp_subevent_signaling_media_codec_sbc_configuration_get_allocation_method(
            packet);

    LOGI("A2DP Source: Received SBC codec configuration, sampling "
         "frequency %u, a2dp_cid 0x%02x, local seid 0x%02x, remote seid "
         "0x%02x.",
         sbc_config.sampling_frequency, cid,
         a2dp_subevent_signaling_media_codec_sbc_configuration_get_local_seid(
             packet),
         a2dp_subevent_signaling_media_codec_sbc_configuration_get_remote_seid(
             packet));

    // Adapt Bluetooth spec definition to SBC Encoder expected input
    sbc_config.allocation_method =
        (btstack_sbc_allocation_method_t)(allocation_method - 1);
    switch (channel_mode) {
    case AVDTP_CHANNEL_MODE_JOINT_STEREO:
      sbc_config.channel_mode = SBC_CHANNEL_MODE_JOINT_STEREO;
      break;
    case AVDTP_CHANNEL_MODE_STEREO:
      sbc_config.channel_mode = SBC_CHANNEL_MODE_STEREO;
      break;
    case AVDTP_CHANNEL_MODE_DUAL_CHANNEL:
      sbc_config.channel_mode = SBC_CHANNEL_MODE_DUAL_CHANNEL;
      break;
    case AVDTP_CHANNEL_MODE_MONO:
      sbc_config.channel_mode = SBC_CHANNEL_MODE_MONO;
      break;
    default:
      btstack_assert(false);
      break;
    }
    dump();
  }

  void begin() override {
    // set encoder parameters
    sbc_codec.setSubbands(sbc_config.subbands);
    sbc_codec.setBitpool(sbc_config.max_bitpool_value);
    sbc_codec.setBlocks(sbc_config.block_length);
    sbc_codec.setAllocationMethod(sbc_config.allocation_method);
  }

  uint8_t *codecCapabilities() override { return media_sbc_codec_capabilities; }
  int codecCapabilitiesSize() override {
    return sizeof(media_sbc_codec_capabilities);
  }
  AudioEncoder &encoder() override { return sbc_codec; }
  void dump() override { sbc_config.dump(); };
  int frameLengthEncoded() override { return sbc_codec.frameLength(); };
  int frameLengthDecoded() override { return sbc_codec.codeSize(); };
  virtual AudioBaseInfo audioInfo() {
    AudioBaseInfo info;
    info.bits_per_sample = 16;
    info.channels = sbc_config.num_channels;
    info.sample_rate = sbc_config.sampling_frequency;
    return info;
  }

  avdtp_media_codec_type_t codecType() {
    return AVDTP_CODEC_SBC;
  }

protected:
  uint8_t media_sbc_codec_configuration[4];
  media_codec_configuration_sbc_t sbc_config;
  SBCEncoder sbc_codec;

  uint8_t media_sbc_codec_capabilities[4] = {
      (AVDTP_SBC_44100 << 4) | AVDTP_SBC_STEREO,
      0xFF, //(AVDTP_SBC_BLOCK_LENGTH_16 << 4) | (AVDTP_SBC_SUBBANDS_8 << 2) |
            // AVDTP_SBC_ALLOCATION_METHOD_LOUDNESS,
      2, 53};
};



/**
 * @brief SBC Decoder implementation
 * @author Phil Schatzmann
 */
class A2DPDecoderSBC : public A2DPDecoder {
public:
  uint8_t *config() override { return media_sbc_codec_configuration; }
  int configSize() override { return sizeof(media_sbc_codec_configuration); }
  void begin() override {}
  uint8_t *codecCapabilities() override { return media_sbc_codec_capabilities; }
  int codecCapabilitiesSize() override {
    return sizeof(media_sbc_codec_capabilities);
  }
  AudioDecoder &decoder() override { return sbc_codec; }

protected:
  uint8_t media_sbc_codec_configuration[4];
  media_codec_configuration_sbc_t sbc_config;
  SBCDecoder sbc_codec;
  uint8_t media_sbc_codec_capabilities[4] = {
      (AVDTP_SBC_44100 << 4) | AVDTP_SBC_STEREO,
      0xFF, //(AVDTP_SBC_BLOCK_LENGTH_16 << 4) | (AVDTP_SBC_SUBBANDS_8 << 2) |
            // AVDTP_SBC_ALLOCATION_METHOD_LOUDNESS,
      2, 53};
};

} // namespace btstack_a2dp