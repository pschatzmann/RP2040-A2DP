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

#if !ENABLE_CLASSIC
#error Bluetooth was not enabled!
#endif

// Common
#define BTSTACK_FILE__ "btstack_a2dp"
#define AVRCP_BROWSING_ENABLED
#define NUM_CHANNELS 2

#define ENABLE_LOG_INFO
#define ENABLE_LOG_ERROR


// Sink
#define MAX_AMPLITUDE_RECEIVED 2500
#define OPTIMAL_FRAMES_MIN 30
#define OPTIMAL_FRAMES_MAX 40
#define ADDITIONAL_FRAMES 20
#define MAX_SBC_FRAME_SIZE 120
//#define ENABLE_AVDTP_ACCEPTOR_EXPLICIT_START_STREAM_CONFIRMATION

// Source
#define MAX_AMPLITUDE_INPUT 32767
#define AUDIO_TIMEOUT_MS 10
#define SBC_STORAGE_SIZE 1030
#define SBC_PACKET_COUNT 5
