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
    LOGI("- bitpool_value [%d, %d]", min_bitpool_value,max_bitpool_value);
  }
};

/**
 * @brief A2DPEncoder: Common Encoder functionality that is needed by A2DP
 * @author Phil Schatzmann
 */
class A2DPEncoder {
public:
  virtual void *config() = 0;
  virtual int configSize() = 0;
  virtual void begin() = 0;
  virtual uint8_t *codecCapabilities() = 0;
  virtual int codecCapabilitiesSize() = 0;
  virtual AudioEncoder &encoder() = 0;
  virtual void dump() = 0;
};

/**
 * @brief A2DPEncoder: Common Decoder functionality that is needed by A2DP
 * @author Phil Schatzmann
 */
class A2DPDecoder {
public:
  virtual void *config() = 0;
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
  void *config() override { return &sbc_config; }

  int configSize() override { return sizeof(sbc_config); }

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

protected:
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
  void *config() override { return &sbc_config; }
  int configSize() override { return sizeof(sbc_config); }
  void begin() override {}
  uint8_t *codecCapabilities() override { return media_sbc_codec_capabilities; }
  int codecCapabilitiesSize() override {
    return sizeof(media_sbc_codec_capabilities);
  }
  AudioDecoder &decoder() override { return sbc_codec; }

protected:
  media_codec_configuration_sbc_t sbc_config;
  SBCDecoder sbc_codec;
  uint8_t media_sbc_codec_capabilities[4] = {
      (AVDTP_SBC_44100 << 4) | AVDTP_SBC_STEREO,
      0xFF, //(AVDTP_SBC_BLOCK_LENGTH_16 << 4) | (AVDTP_SBC_SUBBANDS_8 << 2) |
            // AVDTP_SBC_ALLOCATION_METHOD_LOUDNESS,
      2, 53};
};

} // namespace btstack_a2dp