/**
 * @file dcp.h
 * @author savent (savent_gate@outlook.com)
 * @brief
 * @date 2024-07-07
 * @todo implement response_delay_factor machines
 *
 * Copyright 2023 savent_gate
 *
 */
#pragma once

#include <spn/iface.h>
#include <stdint.h>
/*
 * DCP-PDU
 *   DCP-Unicast-PDU
 *     DCP-Get.Req-PDU
 *     DCP-Set.Req-PDU
 *     DCP-Get-Res-PDU
 *     DCP-Set-Res-PDU
 *     DCP-Identify-Res-PDU
 *   DCP-Multicast-PDU
 *     DCP-Identify-Req-PDU
 *       DCP-IdentifyFilter-Req-PDU
 *       DCP-IdentifyAll-Req-PDU
 *     DCP-Hello-Req-PDU
 */

#define SPN_FRAME_ID_DCP_GET_SET 0xFEFD
#define SPN_FRAME_ID_DCP_MULTICAST_REQUEST 0xFEFE
#define SPN_FRAME_ID_DCP_IDENTIFY_RESPONSE 0xFEFF

#define SPN_DCP_SERVICE_ID_GET 0x03
#define SPN_DCP_SERVICE_ID_SET 0x04
#define SPN_DCP_SERVICE_ID_Identify 0x05
#define SPN_DCP_SERVICE_ID_HELLO 0x06

#define SPN_DCP_SERVICE_TYPE_REQUEST 0x00
#define SPN_DCP_SERVICE_TYPE_RESPONSE 0x01
#define SPN_DCP_SERVICE_TYPE_FAILED_RESPONSE 0x05 /* service_id not supported */

#define SPN_DCP_RESP_DELAY_FACTOR_DISABLED 0x001

#define SPN_DCP_DATA_MAX_LENGTH 1432

#define SPN_DCP_RESPONSE_DELAY_FACTOR_DEFAULT 0x0001

#define SPN_DCP_RESPONSE_DELAY_DEFAULT 400 /* milliseconds, if response_delay_factor equals to 1 */
#define SPN_DCP_RESPONSE_DELAY_MAX 64000 /* milliseconds */

#pragma pack(push, 1)
struct spn_dcp_header {
    uint8_t service_id;
    uint8_t service_type;
    uint16_t xid; /* It shall contain a transaction identification chosen by the client */
    uint32_t response_delay_factor; /* Its reserved as DCP-Unicast-PDU.
                                     * As a DCP-Multicast-PDU, if the responder doesn't have RNG, then the responder MAC address[4:5] equals to
                                       random() % response_delay_factor, mac address[4] is the higher-order octet.
                                       And the minimal response delay equals to 10ms * (random() % response_delay_factor)
                                     * For the requester/client, timeout = 400ms if response_delay_factor equals to 1
                                       and timeout = ceil(1000ms + 10ms * (random() % response_delay_factor) if response_delay_factor > 1, and
                                       limited with SPN_DCP_RESPONSE_DELAY_MAX.
                                     */
    uint16_t dcp_data_length; /* total length of data followed by the DCP-UC-Header or DCP-MC-Header, max 1432 */
};

struct spn_dcp_uc_header {
    uint8_t service_id;
    uint8_t service_type;
    uint16_t xid;
    uint16_t padding;
    uint16_t dcp_data_length;
};
#pragma pack(pop)

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @brief As responder, calculate the minimal response delay
 * 
 * @param rand 
 * @param resp_delay_factor from requester's DCP-Header
 * @return delay in milliseconds
 */
uint16_t spn_dcp_resp_delay(uint16_t rand, uint16_t resp_delay_factor);

/**
 * @brief As requester, calculate the response timeout
 * 
 * @param rand 
 * @param resp_delay_factor from its DCP-Header
 * @return delay in milliseconds
 */
uint16_t spn_dcp_resp_delay_timeout(uint16_t rand, uint16_t resp_delay_factor);

int spn_dcp_input(void* frame, size_t len, uint16_t frame_id, iface_t* iface);

#ifdef __cplusplus
}
#endif
