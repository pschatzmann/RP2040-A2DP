/*
 * Copyright (C) 2014 BlueKitchen GmbH
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

/*
 *  btstack.h
 *  Convenience header to include all public APIs
 */


#ifndef BTSTACK_H
#define BTSTACK_H

#include "btstack_config.h"

#include "btstack/ad_parser.h"
#include "btstack/bluetooth.h"
#include "btstack/bluetooth_psm.h"
#include "btstack/bluetooth_company_id.h"
#include "btstack/bluetooth_data_types.h"
#include "btstack/bluetooth_gatt.h"
#include "btstack/bluetooth_sdp.h"
#include "btstack/btstack_audio.h"
#include "btstack/btstack_control.h"
#include "btstack/btstack_crypto.h"
#include "btstack/btstack_debug.h"
#include "btstack/btstack_defines.h"
#include "btstack/btstack_event.h"
#include "btstack/btstack_hid.h"
#include "btstack/btstack_hid_parser.h"
#include "btstack/btstack_linked_list.h"
#include "btstack/btstack_memory.h"
#include "btstack/btstack_memory_pool.h"
#include "btstack/btstack_network.h"
#include "btstack/btstack_ring_buffer.h"
#include "btstack/btstack_run_loop.h"
#include "btstack/btstack_stdin.h"
#include "btstack/btstack_util.h"
#include "btstack/gap.h"
#include "btstack/hci.h"
#include "btstack/hci_cmd.h"
#include "btstack/hci_dump.h"
#include "btstack/hci_transport.h"
#include "btstack/l2cap.h"
#include "btstack/l2cap_signaling.h"

#ifdef ENABLE_BLE
#include "btstack/ble/att_db.h"
#include "btstack/ble/att_db_util.h"
#include "btstack/ble/att_dispatch.h"
#include "btstack/ble/att_server.h"
#include "btstack/ble/gatt-service/ancs_client.h"
#include "btstack/ble/gatt-service/battery_service_client.h"
#include "btstack/ble/gatt-service/battery_service_server.h"
#include "btstack/ble/gatt-service/battery_service_v1_server.h"
#include "btstack/ble/gatt-service/bond_management_service_server.h"
#include "btstack/ble/gatt-service/cycling_power_service_server.h"
#include "btstack/ble/gatt-service/cycling_speed_and_cadence_service_server.h"
#include "btstack/ble/gatt-service/device_information_service_client.h"
#include "btstack/ble/gatt-service/device_information_service_server.h"
#include "btstack/ble/gatt_service_client.h"
#include "btstack/ble/gatt-service/heart_rate_service_server.h"
#include "btstack/ble/gatt-service/hids_client.h"
#include "btstack/ble/gatt-service/hids_device.h"
#include "btstack/ble/gatt-service/immediate_alert_service_client.h"
#include "btstack/ble/gatt-service/immediate_alert_service_server.h"
#include "btstack/ble/gatt-service/immediate_alert_service_util.h"
#include "btstack/ble/gatt-service/link_loss_service_client.h"
#include "btstack/ble/gatt-service/link_loss_service_server.h"
#include "btstack/ble/gatt-service/link_loss_service_util.h"
#include "btstack/ble/gatt-service/scan_parameters_service_client.h"
#include "btstack/ble/gatt-service/scan_parameters_service_server.h"
#include "btstack/ble/gatt-service/tx_power_service_client.h"
#include "btstack/ble/gatt-service/tx_power_service_server.h"
#include "btstack/ble/gatt_client.h"
#include "btstack/ble/le_device_db.h"
#include "btstack/ble/sm.h"
#endif

#ifdef ENABLE_CLASSIC
#include "btstack/classic/a2dp_sink.h"
#include "btstack/classic/a2dp_source.h"
#include "btstack/classic/avdtp.h"
#include "btstack/classic/avdtp_acceptor.h"
#include "btstack/classic/avdtp_initiator.h"
#include "btstack/classic/avdtp_sink.h"
#include "btstack/classic/avdtp_source.h"
#include "btstack/classic/avdtp_util.h"
#include "btstack/classic/avrcp.h"
#include "btstack/classic/avrcp_browsing.h"
#include "btstack/classic/avrcp_browsing_controller.h"
#include "btstack/classic/avrcp_browsing_target.h"
#include "btstack/classic/avrcp_controller.h"
#include "btstack/classic/avrcp_cover_art_client.h"
#include "btstack/classic/avrcp_media_item_iterator.h"
#include "btstack/classic/avrcp_target.h"
#include "btstack/classic/bnep.h"
#include "btstack/classic/btstack_link_key_db.h"
#include "btstack/classic/btstack_sbc.h"
#include "btstack/classic/btstack_sbc_bluedroid.h"
#include "btstack/classic/device_id_server.h"
#include "btstack/classic/gatt_sdp.h"
#include "btstack/classic/goep_client.h"
#include "btstack/classic/goep_server.h"
#include "btstack/classic/hfp.h"
#include "btstack/classic/hfp_ag.h"
#include "btstack/classic/hfp_hf.h"
#include "btstack/classic/hid_device.h"
#include "btstack/classic/hid_host.h"
#include "btstack/classic/hsp_ag.h"
#include "btstack/classic/hsp_hs.h"
#include "btstack/classic/obex.h"
#include "btstack/classic/pan.h"
#include "btstack/classic/pbap.h"
#include "btstack/classic/pbap_client.h"
#include "btstack/classic/rfcomm.h"
#include "btstack/classic/sdp_client.h"
#include "btstack/classic/rfcomm.h"
#include "btstack/classic/sdp_server.h"
#include "btstack/classic/sdp_util.h"
#include "btstack/classic/spp_server.h"
#endif


#ifdef ENABLE_MESH
#include "btstack/mesh/adv_bearer.h"
#include "btstack/mesh/beacon.h"
#include "btstack/mesh/gatt_bearer.h"
#include "btstack/mesh/gatt-service/mesh_provisioning_service_server.h"
#include "btstack/mesh/gatt-service/mesh_proxy_service_server.h"
#include "btstack/mesh/mesh.h"
#include "btstack/mesh/mesh_access.h"
#include "btstack/mesh/mesh_configuration_client.h"
#include "btstack/mesh/mesh_configuration_server.h"
#include "btstack/mesh/mesh_crypto.h"
#include "btstack/mesh/mesh_foundation.h"
#include "btstack/mesh/mesh_generic_default_transition_time_client.h"
#include "btstack/mesh/mesh_generic_default_transition_time_server.h"
#include "btstack/mesh/mesh_generic_level_client.h"
#include "btstack/mesh/mesh_generic_level_server.h"
#include "btstack/mesh/mesh_generic_model.h"
#include "btstack/mesh/mesh_generic_on_off_client.h"
#include "btstack/mesh/mesh_generic_on_off_server.h"
#include "btstack/mesh/mesh_health_server.h"
#include "btstack/mesh/mesh_iv_index_seq_number.h"
#include "btstack/mesh/mesh_proxy.h"
#include "btstack/mesh/mesh_upper_transport.h"
#include "btstack/mesh/mesh_virtual_addresses.h"
#include "btstack/mesh/pb_adv.h"
#include "btstack/mesh/pb_gatt.h"
#include "btstack/mesh/provisioning.h"
#include "btstack/mesh/provisioning_device.h"
#include "btstack/mesh/provisioning_provisioner.h"
#endif

#endif  // __BTSTACK_H 
