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

#include <spn/config.h>

#include <spn/iface.h>
#include <spn/pdu.h>
#include <stdbool.h>
#include <stddef.h>
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

#define SPN_DCP_SUB_OPT_IP_MAC_ADDRESS 0x01 /* R */
#define SPN_DCP_SUB_OPT_IP_PARAMETER 0x02 /* R/W +F */
#define SPN_DCP_SUB_OPT_IP_FULL_IP_SUITE 0x03 /* R/W/O +F */

#define SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_VENDOR 0x01 /* R +F */
#define SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_NAME_OF_STATION 0x02 /* R/W +F */
#define SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ID 0x03 /* R +F */
#define SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ROLE 0x04 /* R +F */
#define SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_OPTIONS 0x05 /* R +F */
#define SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_ALIAS_NAME 0x06 /* NOTE: Used as filter only +F */
#define SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_INSTANCE 0x07 /* R/O +F */
#define SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_OEM_DEVICE_ID 0x08 /* R/O +F */
#define SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_STANDARD_GATEWAY 0x09 /* R/O +F */
#define SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_RSI_PROPERTIES 0x0A /* R/O */

#define SPN_DCP_SUB_OPT_DHCP_DHCP 0x01 /* R/W/O +F */

#define SPN_DCP_SUB_OPT_CONTROL_START 0x01 /* W */
#define SPN_DCP_SUB_OPT_CONTROL_STOP 0x02 /* W */
#define SPN_DCP_SUB_OPT_CONTROL_SIGNAL 0x03 /* W */
#define SPN_DCP_SUB_OPT_CONTROL_RESPONSE 0x04
#define SPN_DCP_SUB_OPT_CONTROL_FACTORY_RESET 0x05 /* W/O */
#define SPN_DCP_SUB_OPT_CONTROL_RESET_TO_FACTORY 0x06 /* W */

#define SPN_DCP_SUB_OPT_DEVICE_INITIATIVE_DEVICE_INITIATIVE 0x01 /* R +F */

#define SPN_DCP_SUB_OPT_NME_DOMAIN_NME_DOMAIN 0x01 /* R/W +F */
#define SPN_DCP_SUB_OPT_NME_DOMAIN_NME_PRIO 0x02 /* R +F */
#define SPN_DCP_SUB_OPT_NME_DOMAIN_NME_PARAMETER_UUID 0x03 /* R */
#define SPN_DCP_SUB_OPT_NME_DOMAIN_NME_NAME 0x04 /* R +F */
#define SPN_DCP_SUB_OPT_NME_DOMAIN_CIM_INTERFACE 0x05 /* R +F*/

#define SPN_DCP_SUB_OPT_ALL_SELECTOR_ALL_SELECTOR 0xFF /* +F */

#define SPN_DCP_BLOCK_TOUCHED 0x80
#define SPN_DCP_BLOCK_EMPTY 0x00

#define SPN_DCP_COMBINED_OPTION(option, sub_option) ((option << 8) | sub_option)

#define SPN_DCP_BLOCK_TOUCH(block) ((block)->touched |= SPN_DCP_BLOCK_TOUCHED)
#define SPN_DCP_IS_BLOCK_TOUCHED(block) ((block)->touched & SPN_DCP_BLOCK_TOUCHED)
#define SPN_DCP_IS_BLOCK_EMPTY(block) ((block)->touched == SPN_DCP_BLOCK_EMPTY)

/** block_info is only works for option IP_Parameter */
#define SPN_DCP_BLOCK_INFO_NO_IP 0x00
#define SPN_DCP_BLOCK_INFO_STATIC_IP 0x01
#define SPN_DCP_BLOCK_INFO_DHCP_IP 0x02
#define SPN_DCP_BLOCK_INFO_IP_CONFLICT 0x80 /* NOTE: Device detected ip conflict, should handle by controller */

#define SPN_DCP_ROLE_IOD 0x01
#define SPN_DCP_ROLE_IOC 0x02
#define SPN_DCP_ROLE_MULTI_IOD 0x04 /* multiple IO device instance */
#define SPN_DCP_ROLE_SUPERVISOR 0x08

#define SPN_DCP_DHCP_PARAM_VAL_FILTER (61) /* Add Filter for DHCP, possible: MAC, NameOfStation, arbitrary */
#define SPN_DCP_DHCP_PARAM_VAL_POLICY (255) /* DHCP Control */

#define SPN_DCP_DEV_INITIATIVE_ENABLE_HELLO 0x01 /* Device issues a Hello.req after power on */

#define SPN_DCP_DEV_SIGNAL_FLASH_ONCE 0x0100 /* Flash an LED (for example the Ethernet Link LED) or an alternative signaling with duration of 3s with a frequency of, for example, 1Hz */

#pragma pack(push, 1)
struct spn_dcp_header {
    uint8_t service_id;
    uint8_t service_type;
    uint32_t xid; /* It shall contain a transaction identification chosen by the client */
    uint16_t response_delay_factor; /* Its reserved as DCP-Unicast-PDU.
                                     * As a DCP-Multicast-PDU, if the responder doesn't have RNG, then the responder MAC address[4:5] equals to
                                       random() % response_delay_factor, mac address[4] is the higher-order octet.
                                       And the minimal response delay equals to 10ms * (random() % response_delay_factor)
                                     * For the requester/client, timeout = 400ms if response_delay_factor equals to 1
                                       and timeout = ceil(1000ms + 10ms * (random() % response_delay_factor) if response_delay_factor > 1, and
                                       limited with SPN_DCP_RESPONSE_DELAY_MAX.
                                     */
    uint16_t dcp_data_length; /* total length of data followed by the DCP-UC-Header or DCP-MC-Header, max 1432 */
};

struct spn_dcp_general_block {
    uint8_t option;
    uint8_t sub_option;
    uint16_t dcp_block_length; /* additional data length at the end of this struct */
};

struct spn_dcp_all_selector_block {
    struct spn_dcp_general_block base;
};
#pragma pack(pop)

struct spn_dcp_block {
    struct {
        uint16_t block_info;
        uint8_t mac_address[6];
        uint8_t touched;
    } ip_mac;

    struct {
        uint16_t block_info;
        uint32_t ip_addr;
        uint32_t mask;
        uint32_t gw;
        uint8_t touched;
    } ip_param;

    struct {
        uint32_t block_info;
        uint32_t ip_addr;
        uint32_t mask;
        uint32_t gw;
        uint32_t dns_address[4];
        uint8_t touched;
    } ip_full_suit;

    struct {
        char name[32];
        uint8_t name_len;
        uint8_t touched;
    } dev_prop_vendor;

    struct {
        char name[32];
        uint8_t name_len;
        uint8_t touched;
    } dev_prop_name_of_station;

    struct {
        uint16_t vendor_id;
        uint16_t device_id;
        uint8_t touched;
    } dev_prop_device_id;

    struct {
        uint8_t role;
        uint8_t touched;
    } dev_prop_role;

    struct {
        uint16_t options[32]; /* MSB: | option(8) | suboption(8) */
        uint16_t option_num;
        uint8_t touched;
    } dev_prop_device_options;

    struct {
        char alias_name[32];
        uint8_t alias_name_len;
        uint8_t touched;
    } dev_prop_alias_name;

    struct {
        uint16_t instance;
        uint8_t touched;
    } dev_prop_dev_instance;

    struct {
        uint16_t vendor_id;
        uint16_t device_id;
        uint8_t touched;
    } dev_prop_dev_oem_id;

    struct {
        uint16_t standard_gateway;
        uint8_t touched;
    } dev_prop_dev_std_gateway;

    struct {
        uint16_t rsi_prop_value;
        uint8_t touched;
    } dev_prop_rsi_prop;

    struct {
        uint8_t option;
        uint8_t dhcp_param_length;
        uint8_t dhcp_param_data[1]; /* TODO: support flex array */
        uint8_t touched;
    } dhcp;

    struct {
        uint16_t value;
        uint8_t touched;
    } dev_initiative;
};

struct spn_dcp_ident_req {
    uint32_t xid; /* Session id */
    uint16_t option_sub_option[6]; /* MSB: | option(8) | suboption(8) */
};
struct spn_dcp_ident_resp {
    uint32_t xid; /* Session id */
    uint32_t reserved;
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief As responder, calculate the minimal response delay
 *
 * @param rand random number generated by RNG
 * @param resp_delay_factor from requester's DCP-Header
 * @return delay in milliseconds
 */
uint16_t spn_dcp_resp_delay(uint16_t rand, uint16_t resp_delay_factor);

/**
 * @brief As requester, calculate the response timeout
 *
 * @param rand random number generated by RNG
 * @param resp_delay_factor from its DCP-Header
 * @return delay in milliseconds
 */
uint16_t spn_dcp_resp_delay_timeout(uint16_t rand, uint16_t resp_delay_factor);

/**
 * @brief Parse DCP blocks from payload
 *
 * @warning This is a recursive function, it will parse all blocks in the payload.
 * @note For limited depth, add \c deep parameter to control the depth of parsing.
 * @param payload \c spn_dcp_general_type or other dcp_type structure
 * @param len \c payload length
 * @param offset current offset
 * @param deep depth of parsing, 0 for top level.
 * @param[out] ident_block identify results
 * @return int \c SPN_OK on success
 *             \c SPN_EBADMSG on parsing error
 *             \c SPN_EMSGSIZE on depth of recursion exceeds the limit
 */
int spn_dcp_block_parse(void* payload, uint16_t len, uint16_t offset, int deep, struct spn_dcp_block* ident_block);

/**
 * @brief Parse ident.req message
 *
 * @param payload dcp blocks
 * @param len dcp blocks total length
 * @param[out] reqs request list
 * @return \c SPN_OK on success
 *         \c SPN_ENXIO on filter meets mismatch
 *         \c SPN_EBADMSG on parsing error
 *         \c SPN_ENOSYS on not supported feature
 */
int spn_dcp_ident_req_parse(void* payload, uint16_t len, struct spn_dcp_ident_req* reqs);
int spn_dcp_ident_req_assemble(struct eth_hdr* hw_hdr, struct spn_dcp_ident_resp* resp, iface_t* iface);
int spn_dcp_ident_resp_parse(void* payload, uint16_t len, struct spn_dcp_ident_resp* resp);
int spn_dcp_ident_resp_assemble(struct eth_hdr* hw_hdr, struct spn_dcp_ident_req* reqs, iface_t* iface);
int spn_dcp_hello_req_parse(void* payload, uint16_t len, struct spn_dcp_ident_req* reqs);
int spn_dcp_hello_req_assemble(struct eth_hdr* hw_hdr, struct spn_dcp_ident_req* reqs);

/**
 * @defgroup dcp_filter DCP filter
 * @{
 */
bool spn_dcp_filter_ip(uint32_t ip, uint32_t mask, uint32_t gw);
bool spn_dcp_filter_dns(uint32_t dns1, uint32_t dns2, uint32_t dns3, uint32_t dns4);
bool spn_dcp_filter_station_of_name(const char* name, uint16_t len);
bool spn_dcp_filter_alias(const char* name, uint16_t len);
bool spn_dcp_filter_vendor_name(const char* name, uint16_t len);
bool spn_dcp_filter_vendor_id(uint16_t vendor_id, uint16_t device_id);
bool spn_dcp_filter_oem_id(uint16_t vendor_id, uint16_t device_id);
bool spn_dcp_filter_role(uint8_t role);
bool spn_dcp_filter_options(const uint16_t* options, uint16_t num);
bool spn_dcp_filter_instance(uint16_t instance);
bool spn_dcp_filter_device_initiative(uint16_t value);
/**
 * @}
 */

/**
 * @defgroup dcp_pack DCP pack
 * @{
 */
void spn_dcp_pack_block(void* dest, uint16_t option_sub_option, uint16_t payload_len, uint16_t block_info);
void spn_dcp_pack_ip(void* dest, iface_t* iface);
void spn_dcp_pack_dns(void* dest, iface_t* iface);
int spn_dcp_pack_station_of_name(void* dest);
int spn_dcp_pack_alias(void* dest);
int spn_dcp_pack_vendor_name(void* dest);
void spn_dcp_pack_device_id(void* dest);
void spn_dcp_pack_oem_id(void* dest);
void spn_dcp_pack_role(void* dest);
int spn_dcp_pack_options(void* dest);
void spn_dcp_pack_instance(void* dest);
void spn_dcp_pack_device_initiative(void* dest);
/**
 * @}
 */

/**
 * @brief main entry of dcp protocol
 *
 * @param frame payload of RTC frame
 * @param len   payload length of RTC frame
 * @param frame_id PN frame id
 * @param hw_hdr ethernet header
 * @param iface net interface
 * @return \c SPN_OK on success
 */
int spn_dcp_input(void* frame, size_t len, uint16_t frame_id, struct eth_hdr* hw_hdr, iface_t* iface);

/**
 * @brief Send DCP Identify request
 *
 * @return \c SPN_OK on success
 */
int spn_dcp_discovery_packet(void* payload, size_t max_len, const struct spn_dcp_block* blocks);

int spn_dcp_resp_ident(uint32_t xid, uint16_t resp_delay_factor, uint8_t* dst_mac, iface_t* iface);

#ifdef __cplusplus
}
#endif
