
// btstack_config.h for most tests
//

#ifndef BTSTACK_CONFIG_H
#define BTSTACK_CONFIG_H

// Port related features
#define HAVE_BTSTACK_STDIN
#define HAVE_MALLOC
//#define HAVE_POSIX_FILE_IO
//#define HAVE_POSIX_TIME
// We don't give btstack a malloc, so use a fixed-size ATT DB.
//#define MAX_ATT_DB_SIZE 1024*2


#define HAVE_EMBEDDED_TIME_MS


// BTstack features that can be enabled
#ifndef ENABLE_BLE
#define ENABLE_BLE
#endif
#ifndef ENABLE_CLASSIC
#define ENABLE_CLASSIC
#endif
#define ENABLE_GATT_CLIENT_PAIRING
#define ENABLE_LOG_ERROR
#define ENABLE_LOG_INFO
#define ENABLE_PRINTF_HEXDUMP
#define ENABLE_SDP_DES_DUMP
#define ENABLE_SDP_EXTRA_QUERIES

// #define ENABLE_LE_SECURE_CONNECTIONS
#define ENABLE_L2CAP_ENHANCED_RETRANSMISSION_MODE
#define ENABLE_LE_CENTRAL
#define ENABLE_LE_PERIPHERAL
#define ENABLE_LE_SIGNED_WRITE
//#define ENABLE_SDP_EXTRA_QUERIES
//#define ENABLE_AVCTP_FRAGMENTATION

// BTstack configuration. buffers, sizes, ...
#define HCI_ACL_PAYLOAD_SIZE 1024
#define HCI_INCOMING_PRE_BUFFER_SIZE 6
#define NVM_NUM_DEVICE_DB_ENTRIES 4
#define NVM_NUM_LINK_KEYS 2

#define SDP_SERVICE_NAME_LEN 20

// #define MAX_NR_MESH_SUBNETS            2
// #define MAX_NR_MESH_TRANSPORT_KEYS    16
// #define MAX_NR_MESH_VIRTUAL_ADDRESSES 16

// // allow for one NetKey update
// #define MAX_NR_MESH_NETWORK_KEYS      (MAX_NR_MESH_SUBNETS+1)
// #define MESH_DEVICE_KEY_INDEX 0xffff

#endif
