/**
 * @file dcp.h
 * @author savent (savent_gate@outlook.com)
 * @brief
 * @date 2024-07-07
 * @todo mac filter need to be implemented
 *
 * Copyright 2023 savent_gate
 *
 */
#pragma once

#include <spn/iface.h>
#include <stdbool.h>
#include <stdint.h>
#include <lwip/prot/ethernet.h>
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
#define SPN_DCP_DATA_MAX_LENGTH 1432

#define SPN_DCP_RESP_DELAY_FACTOR_DISABLED 0x001
#define SPN_DCP_RESPONSE_DELAY_FACTOR_DEFAULT 0x0001
#define SPN_DCP_RESPONSE_DELAY_DEFAULT 400 /* milliseconds, if response_delay_factor equals to 1 */
#define SPN_DCP_RESPONSE_DELAY_MAX 64000 /* milliseconds */

#define SPN_DCP_SERVICE_ID_GET 0x03
#define SPN_DCP_SERVICE_ID_SET 0x04
#define SPN_DCP_SERVICE_ID_IDENTIFY 0x05
#define SPN_DCP_SERVICE_ID_HELLO 0x06

#define SPN_DCP_SERVICE_TYPE_REQUEST 0x00
#define SPN_DCP_SERVICE_TYPE_RESPONSE 0x01
#define SPN_DCP_SERVICE_TYPE_FAILED_RESPONSE 0x05 /* service_id not supported */

#define SPN_DCP_OPTION_IP 0x01
#define SPN_DCP_OPTION_DEVICE_PROPERTIES 0x02
#define SPN_DCP_OPTION_DHCP 0x03
#define SPN_DCP_OPTION_CONTROL 0x05
#define SPN_DCP_OPTION_DEVICE_INITIATIVE 0x06
#define SPN_DCP_OPTION_NME_DOMAIN 0x07
#define SPN_DCP_OPTION_MANUFACTURE_SPEC_BEGIN 0x80
#define SPN_DCP_OPTION_MANUFACTURE_SPEC_END 0xFE
#define SPN_DCP_OPTION_ALL_SELECTOR 0xFF

#define SPN_DCP_SUB_OPT_IP_MAC_ADDRESS 0x01
#define SPN_DCP_SUB_OPT_IP_PARAMETER 0x02
#define SPN_DCP_SUB_OPT_IP_FULL_IP_SUITE 0x03

#define SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_VENDER 0x01
#define SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_NAME_OF_STATION 0x02
#define SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ID 0x03
#define SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ROLE 0x04
#define SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_OPTIONS 0x05
#define SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_ALIAS_NAME 0x06
#define SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_INSTANCE 0x07
#define SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_OEM_DEVICE_ID 0x08
#define SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_STANDARD_GATEWAY 0x09
#define SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_RSI_PROPERTIES 0x0A

#define SPN_DCP_SUB_OPT_DHCP_DHCP 0x01

#define SPN_DCP_SUB_OPT_CONTROL_START 0x01
#define SPN_DCP_SUB_OPT_CONTROL_STOP 0x02
#define SPN_DCP_SUB_OPT_CONTROL_SIGNAL 0x03
#define SPN_DCP_SUB_OPT_CONTROL_RESPONSE 0x04
#define SPN_DCP_SUB_OPT_CONTROL_FACTORY_RESET 0x05
#define SPN_DCP_SUB_OPT_CONTROL_RESET_TO_FACTORY 0x06

#define SPN_DCP_SUB_OPT_DEVICE_INITIATIVE_DEVICE_INITIATIVE 0x01

#define SPN_DCP_SUB_OPT_NME_DOMAIN_NME_DOMAIN 0x01
#define SPN_DCP_SUB_OPT_NME_DOMAIN_NME_PRIO 0x02
#define SPN_DCP_SUB_OPT_NME_DOMAIN_NME_PARAMETER_UUID 0x03
#define SPN_DCP_SUB_OPT_NME_DOMAIN_NME_NAME 0x04
#define SPN_DCP_SUB_OPT_NME_DOMAIN_CIM_INTERFACE 0x05

#define SPN_DCP_SUB_OPT_ALL_SELECTOR_ALL_SELECTOR 0x01

#define SPN_DCP_COMBINED_OPTION(option, sub_option) ((option << 8) | sub_option)

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

struct spn_dcp_general_type {
    uint8_t option;
    uint8_t sub_option;
    uint16_t dcp_block_length; /* additional data length at the end of this struct */
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

/**
 * @brief Check if service id supports multicast mac address
 *
 * @param service_id
 * @note only Ident.req and Hello.req support multicast mac address
 * @return true Support multicast mac address
 * @return false Not support multicast mac address
 */
bool spn_dcp_support_multicast(uint8_t service_id, uint8_t service_type);

int spn_dcp_input(void* frame, size_t len, uint16_t frame_id, struct eth_hdr *hw_hdr, iface_t* iface);

int spn_dcp_resp_ident(uint32_t xid, uint16_t resp_delay_factor, iface_t* iface);

#ifdef __cplusplus
}
#endif
