/**
 * @file dcp.h
 * @author savent (savent_gate@outlook.com)
 * @brief
 * @date 2024-07-07
 *
 * @todo timeout mechanism to be implemented
 * @todo get/set mechanism to be implemented
 * @todo set user callback to be implemented
 * @todo factory reset to be implemented
 * @todo reset to factory to be implemented
 * @todo hello to be implemented
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
#define SPN_DCP_DATA_MIN_LENGTH SPN_RTC_MINIMAL_FRAME_SIZE

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

#define SPN_DCP_BLOCK_TOUCH(block) ((block)->touched |= SPN_DCP_BLOCK_TOUCHED)
#define SPN_DCP_IS_BLOCK_TOUCHED(block) ((block)->touched & SPN_DCP_BLOCK_TOUCHED)
#define SPN_DCP_IS_BLOCK_EMPTY(block) ((block)->touched == SPN_DCP_BLOCK_EMPTY)
#define BLOCK_TYPE(option, sub_option) ((option << 8) | sub_option)
#define GET_VALUE(ptr, type, offset) (*(type*)((uintptr_t)ptr + offset))

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

struct spn_dcp_block_req {
    uint32_t xid; /* Session id */
    uint16_t option_sub_option[6]; /* MSB: | option(8) | suboption(8) */
};

struct spn_dcp_db {
    uint32_t xid; /* Session id */

    uint16_t block_info;

    uint8_t mac_addr[6];
    uint32_t ip_addr;
    uint32_t ip_mask;
    uint32_t ip_gw;
    uint32_t ip_dns[4];
    char* name_of_station;
    char* name_of_vendor;
    char* name_of_alias;

    uint16_t vendor_id;
    uint16_t device_id;

    uint16_t oem_vendor_id;
    uint16_t oem_device_id;

    uint16_t* options; /* MSB: | option(8) | sub_option(8), end with (0,0) */

    uint16_t dev_instance;
    uint16_t rsi_prop_value;
    uint8_t device_role;
    uint8_t std_gw_flag;
    uint8_t dhcp_option;
    uint8_t dhcp_param_len;
    uint8_t* dhcp_param;

    uint16_t dev_initiative_flag;
};

enum spn_dcp_dev_state {
    dcp_dev_state_hollow,
    dcp_dev_state_ident,
    dcp_dev_state_query,
    dcp_dev_state_active
};

struct spn_dcp_dev_session {
    enum spn_dcp_dev_state state;
    struct spn_dcp_db resp;
};

struct spn_dcp_ctx {
    struct spn_dcp_dev_session dev_session[8];
    uint32_t xid_base; /* used when try to start a conversation */
    iface_t* interface;
};

enum spn_ip_status {
    ip_status_none = SPN_DCP_BLOCK_INFO_NO_IP,
    ip_status_static = SPN_DCP_BLOCK_INFO_STATIC_IP,
    ip_status_auto = SPN_DCP_BLOCK_INFO_DHCP_IP,
    ip_status_conflict = SPN_DCP_BLOCK_INFO_IP_CONFLICT
};

enum spn_role {
    role_iod = SPN_DCP_ROLE_IOD,
    role_ioc = SPN_DCP_ROLE_IOC,
    role_mult_iod = SPN_DCP_ROLE_MULTI_IOD,
    role_supervisor = SPN_DCP_ROLE_SUPERVISOR
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
 * @brief Find the next block offset
 *
 * @param payload payload points to the first block
 * @param offset payload offset
 * @return int new offset
 */
static inline int spn_dcp_block_walk(void* payload, int offset)
{
    struct spn_dcp_general_block* gen_block;
    uint16_t block_len;
    gen_block = (struct spn_dcp_general_block*)((uint8_t*)payload + offset);
    block_len = lwip_htons(gen_block->dcp_block_length);
    offset += sizeof(*gen_block) + block_len;
    offset = (offset + 1) & ~1; /* align to 2 bytes */
    return offset;
}

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
int spn_dcp_ident_req_parse(void* payload, uint16_t len, struct spn_dcp_block_req* reqs, iface_t* iface);
int spn_dcp_ident_req_assemble(const uint16_t* options, struct spn_dcp_db* resp, iface_t* iface);
int spn_dcp_ident_resp_parse(void* payload, uint16_t len, struct spn_dcp_db* resp);
int spn_dcp_ident_resp_assemble(struct eth_hdr* hw_hdr, struct spn_dcp_block_req* reqs, iface_t* iface);
int spn_dcp_set_req_parse(void* payload, uint16_t len, iface_t* iface);
int spn_dcp_hello_req_parse(void* payload, uint16_t len, struct spn_dcp_block_req* reqs);
int spn_dcp_hello_req_assemble(struct eth_hdr* hw_hdr, struct spn_dcp_block_req* reqs);

/**
 * @defgroup dcp_filter DCP filter
 * @{
 */
bool spn_dcp_filter_ip(iface_t* iface, uint32_t ip, uint32_t mask, uint32_t gw);
bool spn_dcp_filter_dns(iface_t* iface, uint32_t dns1, uint32_t dns2, uint32_t dns3, uint32_t dns4);
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
 * @note to be aware, all parameter is in network byte order
 * @{
 */
void spn_dcp_pack_resp_block(void* dest, uint16_t option_sub_option, uint16_t payload_len, uint16_t block_info);
void spn_dcp_pack_req_block(void* dest, uint16_t option_sub_option, uint16_t payload_len);
void spn_dcp_pack_ip(void* dest, uint32_t ip, uint32_t mask, uint32_t gw);
void spn_dcp_pack_dns(void* dest, uint32_t dns1, uint32_t dns2, uint32_t dns3, uint32_t dns4);
int spn_dcp_pack_station_of_name(void* dest, const char* name);
int spn_dcp_pack_alias(void* dest, const char* name);
int spn_dcp_pack_vendor_name(void* dest, const char* name);
void spn_dcp_pack_device_id(void* dest, uint16_t vendor_id, uint16_t device_id);
void spn_dcp_pack_oem_id(void* dest, uint16_t vendor_id, uint16_t device_id);
void spn_dcp_pack_role(void* dest, enum spn_role role);
int spn_dcp_pack_options(void* dest, const uint16_t* options);
void spn_dcp_pack_instance(void* dest, uint16_t instance);
void spn_dcp_pack_device_initiative(void* dest, uint16_t initiative);
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
int spn_dcp_input(void* frame, uint16_t len, uint16_t frame_id, struct eth_hdr* hw_hdr, iface_t* iface);

/**
 * @brief Trigger to search all devices
 *
 * @note This function will send a multicast identify request to all devices,
 *       and must called outsize of TCPIP thread
 * @todo add timeout mechanism
 * @return int
 */
int spn_dcp_search_all(void);
int spn_dcp_search_name_of_station(char* name);
int spn_dcp_search_name_of_vendor(char* name);
int spn_dcp_search_name_of_alias(char* name);

/**
 * @brief Initialize DCP context
 *
 */
void spn_dcp_init(iface_t* iface);

/**
 * @brief Get DCP context
 *
 * @return struct spn_dcp_ctx*
 */
struct spn_dcp_ctx* spn_dcp_get_ctx(void);

#ifdef __cplusplus
}
#endif
