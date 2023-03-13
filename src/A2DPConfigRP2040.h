/**
 * @file A2DPConfigRP2040.h
 * @author Phil Schatzmann
 * @brief A2DP Configuration
 * @version 0.1
 * @date 2023-03-13
 * 
 * @copyright Copyright (c) 2023
 * 
 */
#pragma once

#ifndef ENABLE_BLE
#error Enable Bluetooth
#endif

// Common
#define BTSTACK_FILE__ "a2dp_rp2040"
#define AVRCP_BROWSING_ENABLED
#define NUM_CHANNELS 2
//#define HAVE_BTSTACK_STDIN

// Sink
#define OPTIMAL_FRAMES_MIN 30
#define OPTIMAL_FRAMES_MAX 40
#define ADDITIONAL_FRAMES 20
#define MAX_SBC_FRAME_SIZE 120
#define BYTES_PER_FRAME     (2*NUM_CHANNELS)
//#define ENABLE_AVDTP_ACCEPTOR_EXPLICIT_START_STREAM_CONFIRMATION

// Source
#define VOLUME_REDUCTION 0
#define BYTES_PER_AUDIO_SAMPLE (2 * NUM_CHANNELS)
#define AUDIO_TIMEOUT_MS 10
#define TABLE_SIZE_441HZ 100
#define SBC_STORAGE_SIZE 1030
