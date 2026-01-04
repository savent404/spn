#pragma once

#include <spn/config.h>
#include <spn/db.h>
#include <spn/errno.h>
#include <spn/pdu.h>

#include <netif/ethernet.h>
#include <spn/lwip_spec.h>
#include <spn/iface.h>

#ifndef SPN_DCP_MAX_SIZE
#define SPN_DCP_MAX_SIZE (SPN_RTC_MAXIMAL_FRAME_SIZE - SPN_PDU_HDR_SIZE)
#endif

#ifndef SPN_DCP_MIN_SIZE
#define SPN_DCP_MIN_SIZE (SPN_RTC_MINIMAL_FRAME_SIZE - SPN_PDU_HDR_SIZE)
#endif

/**
 * @brief timeout of set/get request
 */
#ifndef SPN_DCP_UC_TIMEOUT
#define SPN_DCP_UC_TIMEOUT 1000
#endif

#define DCP_MCS_MAC_ADDR \
  { 0x01, 0x0E, 0xCF, 0x00, 0x00, 0x00 }
#define DCP_MCS_MAC_FILTER_BEGIN (0x000020)
#define DCP_MCS_MAC_FILTER_END (0x00003F)

#ifndef BLOCK_TYPE
#define BLOCK_TYPE(option, sub_option) ((option) << 8 | (sub_option))
#endif

#ifndef PTR_OFFSET
#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))
#endif

enum dcp_service_id { DCP_SRV_ID_GET = 0x03, DCP_SRV_ID_SET = 0x04, DCP_SRV_ID_IDENT = 0x05, DCP_SRV_ID_HELLO = 0x06 };

enum dcp_service_type { DCP_SRV_TYPE_REQ = 0x00, DCP_SRV_TYPE_RES = 0x01, DCP_SRV_TYPE_ERR = 0x05 };

enum dcp_qualifer_type { DCP_QUALIFER_TEMP = 0x00, DCP_QUALIFER_PERSISTENT = 0x01 };

enum dcp_signal_type { DCP_SIGNAL_LED_FLASH = 0x0004 };

enum dcp_option {
  DCP_OPT_IP = 0x01,
  DCP_OPT_DEV_PROP = 0x02,
  DCP_OPT_DHCP = 0x03,
  DCP_OPT_CONTROL = 0x05,
  DCP_OPT_DEV_INITIATIVE = 0x06,
  DCP_OPT_NME_DOMAIN = 0x07,
  DCP_OPT_ALL_SELECTOR = 0xFF
};

enum dcp_sub_option_ip { DCP_SUB_OPT_IP_MAC = 0x01, DCP_SUB_OPT_IP_PARAM = 0x02, DCP_SUB_OPT_IP_FULL_SUITE = 0x03 };

enum dcp_sub_option_dev_prop {
  DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR = 0x01,
  DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION = 0x02,
  DCP_SUB_OPT_DEV_PROP_DEVICE_ID = 0x03,
  DCP_SUB_OPT_DEV_PROP_DEVICE_ROLE = 0x04,
  DCP_SUB_OPT_DEV_PROP_DEVICE_OPTIONS = 0x05,
  DCP_SUB_OPT_DEV_PROP_NAME_OF_ALIAS = 0x06,
  DCP_SUB_OPT_DEV_PROP_DEVICE_INSTANCE = 0x07,
  DCP_SUB_OPT_DEV_PROP_OEM_DEVICE_ID = 0x08,
  DCP_SUB_OPT_DEV_PROP_STD_GATEWAY = 0x09,
  DCP_SUB_OPT_DEV_PROP_RSI = 0x0A
};

/* TODO: TBD */
enum dcp_sub_option_dhcp {
  DCP_SUB_OPT_DHCP_CLIENT_IDENT = 0x3d,
};

enum dcp_sub_option_ctr {
  DCP_SUB_OPT_CTRL_START = 0x01,
  DCP_SUB_OPT_CTRL_STOP = 0x02,
  DCP_SUB_OPT_CTRL_SIGNAL = 0x03,
  DCP_SUB_OPT_CTRL_RESPONSE = 0x04,
  DCP_SUB_OPT_CTRL_FACTORY_RESET = 0x05,
  DCP_SUB_OPT_CTRL_RESET_TO_FACTORY = 0x06
};

enum dcp_sub_option_dev_initiative { DCP_SUB_OPT_DEV_INITIATIVE = 0x01 };

enum dcp_sub_option_nme {
  DCP_SUB_OPT_NME_DOMAIN = 0x01,
  DCP_SUB_OPT_NME_PRIORITY = 0x02,
  DCP_SUB_OPT_NME_PARAM_UUID = 0x03,
  DCP_SUB_OPT_NME_NAME = 0x04,
  DCP_SUB_OPT_NME_CIM_INTERFACE = 0x05,
};

enum dcp_sub_option_all_selector { DCP_SUB_OPT_ALL_SELECTOR = 0xFF };

enum dcp_block_error {
  DCP_BLOCK_ERR_OK = 0,
  DCP_BLOCK_ERR_OPTION_NOT_SUPPORTED = 1,
  DCP_BLOCK_ERR_SUB_OPTION_NOT_SUPPORTED = 2, /* suboption, dataset or qualifier not supported */
  DCP_BLOCK_ERR_SUB_OPTION_NOT_SET = 3,       /* if suboption could not be set
                                               * for local reasons / wrong semantics / write access not defined */
  DCP_BLOCK_ERR_RESOURCE_ERR = 4,             /* if there is a temporary resource rror whithin the server */
  DCP_BLOCK_ERR_LOCAL_ERR = 5,                /* if set service is not possible for local rason like: ProtectionOn=true,
                                               * RejectDcpSetRequests activated by an established AR*/
  DCP_BLOCK_ERR_OPERATION_ERR = 6,            /* If set service is not possible because of application operation, like
                                               * AR established, local rules disallow reset */
};

/**
 * DCP Bitmap of option/sub option
 *
 * @note This order is used to generate right sequence of req/rsp, do not change it
 */
enum dcp_bitmap_idx {
  DCP_BIT_IDX_CTRL_START = 0,
  DCP_BIT_IDX_ALL_SELECTOR,
  DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION,
  DCP_BIT_IDX_DEV_PROP_NAME_OF_ALIAS,
  DCP_BIT_IDX_IP_MAC_ADDRESS,
  DCP_BIT_IDX_IP_PARAMETER,
  DCP_BIT_IDX_IP_FULL_IP_SUITE,
  DCP_BIT_IDX_DEV_PROP_NAME_OF_VENDOR,
  DCP_BIT_IDX_DEV_PROP_DEVICE_ID,
  DCP_BIT_IDX_DEV_PROP_DEVICE_ROLE,
  DCP_BIT_IDX_DEV_PROP_DEVICE_OPTIONS,
  DCP_BIT_IDX_DEV_PROP_DEVICE_INSTANCE,
  DCP_BIT_IDX_DEV_PROP_OEM_DEVICE_ID,
  DCP_BIT_IDX_DEV_PROP_STANDARD_GATEWAY,
  DCP_BIT_IDX_DEV_PROP_RSI_PROPERTIES,
  DCP_BIT_IDX_DHCP_CLIENT_IDENT,
  DCP_BIT_IDX_CTRL_SIGNAL,
  DCP_BIT_IDX_CTRL_RESPONSE,
  DCP_BIT_IDX_CTRL_FACTORY_RESET,
  DCP_BIT_IDX_CTRL_RESET_TO_FACTORY,
  DCP_BIT_IDX_DEVICE_INITIATIVE,
  DCP_BIT_IDX_NME_DOMAIN_NME_DOMAIN,
  DCP_BIT_IDX_NME_DOMAIN_NME_PRIO,
  DCP_BIT_IDX_NME_DOMAIN_NME_PARAMETER_UUID,
  DCP_BIT_IDX_NME_DOMAIN_NME_NAME,
  DCP_BIT_IDX_NME_DOMAIN_CIM_INTERFACE,
  DCP_BIT_IDX_CTRL_STOP,
  DCP_BIT_IDX_NUM
};

/*@ 
  @ // 这些公理提供编译时常量的数学约束，帮助证明器避免超时
  @ // DCP_BIT_IDX_NUM 实际值为 27，最大偏移量为 12 + 54 = 66 < 56（最小帧）  
  @ axiomatic DCP_Bounds {
  @   axiom dcp_bit_idx_num_value: DCP_BIT_IDX_NUM == 27;
  @   axiom dcp_bit_idx_num_positive: DCP_BIT_IDX_NUM > 0;
  @   axiom dcp_bit_idx_num_upper: DCP_BIT_IDX_NUM <= 32;
  @   axiom offset_hdr_value: SPN_PDU_HDR_SIZE + 10 == 12;
  @   axiom max_offset_bound: 12 + 2 * 27 <= 56;
  @ }
  @*/

/**
 * Attributes of option/sub option
 *
 * | Sub Option                              | Read | Write | Optional | Filter |
 * |-----------------------------------------|------|-------|----------|--------|
 * | IP_MAC_ADDRESS                          | ( )  |       |          |        |
 * | IP_PARAMETER                            | (x)  | (x)   |          |  ( )   |
 * | IP_FULL_IP_SUITE                        | ( )  | ( )   |   ( )    |  ( )   |
 * | DEVICE_PROPERTIES_NAME_OF_VENDOR        | (x)  |       |          |  (x)   |
 * | DEVICE_PROPERTIES_NAME_OF_STATION       | (x)  | (x)   |          |  (x)   |
 * | DEVICE_PROPERTIES_DEVICE_ID             | (x)  |       |          |  (x)   |
 * | DEVICE_PROPERTIES_DEVICE_ROLE           | (x)  |       |          |  ( )   |
 * | DEVICE_PROPERTIES_DEVICE_OPTIONS        | (x)  |       |          |  ( )   |
 * | DEVICE_PROPERTIES_NAME_OF_ALIAS         |      |       |          |  (x)   |
 * | DEVICE_PROPERTIES_DEVICE_INSTANCE       | ( )  |       |          |  ( )   |
 * | DEVICE_PROPERTIES_OEM_DEVICE_ID         | ( )  |       |          |  ( )   |
 * | DEVICE_PROPERTIES_STANDARD_GATEWAY      | ( )  |       |          |  ( )   |
 * | DEVICE_PROPERTIES_RSI_PROPERTIES        | ( )  |       |          |  ( )   |
 * | DHCP_DHCP                               | ( )  | ( )   |   ( )    |  ( )   |
 * | CONTROL_START                           |      | ( )   |          |        |
 * | CONTROL_STOP                            |      | ( )   |          |        |
 * | CONTROL_SIGNAL                          |      | (x)   |          |        |
 * | CONTROL_RESPONSE                        |      |       |          |        |
 * | CONTROL_FACTORY_RESET                   |      | ( )   |   ( )    |        |
 * | CONTROL_RESET_TO_FACTORY                |      | (x)   |          |        |
 * | DEVICE_INITIATIVE_DEVICE_INITIATIVE     | ( )  |       |          |  ( )   |
 * | NME_DOMAIN_NME_DOMAIN                   | ( )  | ( )   |          |  ( )   |
 * | NME_DOMAIN_NME_PRIO                     | ( )  |       |          |  ( )   |
 * | NME_DOMAIN_NME_PARAMETER_UUID           | ( )  |       |          |        |
 * | NME_DOMAIN_NME_NAME                     | ( )  |       |          |  ( )   |
 * | NME_DOMAIN_CIM_INTERFACE                | ( )  |       |          |  ( )   |
 * | ALL_SELECTOR                            |      |       |          |  (x)   |
 * +----------------------------------------------------------------------------+
 *
 * DCP Payloads
 * @defgroup dcp_payloads DCP Payloads
 * @{
 */
#pragma pack(push, 2)

/**
 * @brief dcp header
 *
 * | Field          | Size | Description             |
 * |----------------|------|-------------------------|
 * | service_id     | 1    | Service ID              |
 * | service_type   | 1    | Service Type            |
 * | xid            | 4    | Gen by requester        |
 * | response_delay | 2    | Response Delay, default |
 * | data_length    | 2    | Data Length             |
 * +-------------------------------------------------+
 */
struct dcp_header {
  uint8_t service_id;
  uint8_t service_type;
  uint16_t xid_high; /* NOTE: xid(u32) will cause alignment issue */
  uint16_t xid_low;
  uint16_t response_delay;
  uint16_t data_length;
};

/**
 * @brief dcp block
 *
 * @note For dcp block, the length is not fixed, it is calculated by the length of data
 * @note As response block, the first word of data is block_info or reserved(if option is not \c DCP_OPT_IP)
 *
 * | Field          | Size | Description             |
 * |----------------|------|-------------------------|
 * | option         | 1    | Option                  |
 * | sub_option     | 1    | Sub Option              |
 * | length         | 2    | Length of data          |
 * | data           | n    | Data                    |
 * +-------------------------------------------------+
 */
struct dcp_block_hdr {
  uint8_t option;
  uint8_t sub_option;
  uint16_t length;
  char data[0]; /* NOTE: Clang told me it has to be chared */
};
#pragma pack(pop)

/**
 * @} end of dcp_payloads
 */

/*********************************************
 * DCP internal structures
 * @defgroup dcp_internal DCP Internal
 */

/**
 * DCP state machine
 *
 * @note This state machine is used to track
 *       the state of DCP serval request/response
 */
enum dcp_state {
  DCP_STATE_IDLE = 0,

  DCP_STATE_IDENT_REQ,
  DCP_STATE_IDENT_IND,
  DCP_STATE_IDENT_RES,
  DCP_STATE_IDENT_CNF,

  DCP_STATE_GET_REQ,
  DCP_STATE_GET_IND,
  DCP_STATE_GET_RES,
  DCP_STATE_GET_CNF,

  DCP_STATE_SET_REQ,
  DCP_STATE_SET_IND,
  DCP_STATE_SET_RES,
  DCP_STATE_SET_CNF,

  DCP_STATE_HELLO_REQ,
  DCP_STATE_HELLO_IND,
};

/**
 * Unicast sender context
 *
 * @note UCS is a stateful context, it is used to track the state of unicast sender
 * @note this context is used for handle set/get.req & set/get.cnf
 *
 * IDLE -> IDLE: xid missmatched/in operating mode
 * IDLE -> SET_REQ: send set.req
 * SET_REQ -> SET_REQ: some of device triggered set.cnf
 * SET_REQ -> SET_REQ: xid missmatched
 * SET_REQ -> IDLE: timeout
 */
struct dcp_ucs_ctx {
  uint32_t xid;                                      /* session token, it will increase by state machine go to idle */
  uint32_t req_options_bitmap;                       /* set bits if option is requested */
  uint16_t req_qualifier_bitmap;                     /* set bits if persistent mode is requested
                                                      * @note reset to factory and factory reset are special qualifier
                                                      */
  enum dcp_block_error resp_errors[DCP_BIT_IDX_NUM]; /* responsed erros, only the requested option be valid */
  uint16_t ex_ifr;                                   /* Indicate the external interface id */
  enum dcp_state state;

  /* set if option is requested */
  /* TODO: still filter not implemented */
  uint32_t ip_addr;                   /* BE */
  uint32_t ip_mask;                   /* BE */
  uint32_t ip_gw;                     /* BE */
  uint16_t factory_reset_qualifer;    /* special qualifier cna't be restored in bitmap */
  uint16_t reset_to_factory_qualifer; /* special qualifier can't be restored in bitmap */
  const char* station_name;
};

/**
 * Unicast receiver context
 *
 * @note UCR is a none-stateful context, it is used to store some temporary variables
 * @note this context is used for handle set/get.ind & set/get.rsp
 */
struct dcp_ucr_ctx {
  uint32_t xid;
  uint32_t req_options_bitmap;                 /* set if option is requested */
  uint16_t req_qualifier_bitmap;               /* set if persistent mode is requested */
  enum dcp_block_error error[DCP_BIT_IDX_NUM]; /* errors need to report to requester */
};

struct dcp_ctx;

/**
 * Multicast sender context
 *
 * @note MCS is a stateful context, it is used to track the state of multicast sender
 * @note this context is used for handle ident.req & ident.cnf
 *
 * IDLE -> IDENT_REQ: send ident.req
 * IDLE -> IDLE: xid missmatched/in operating mode
 * IDENT_REQ -> IDENT_REQ: some of device triggered ident.cnf
 * IDENT_REQ -> IDLE: timeout, no response from device should be handled
 */
struct dcp_mcs_ctx {
  /* inputs */
  uint32_t xid;
  uint32_t req_options_bitmap;
  uint16_t response_delay_factory; /* set 1 if we are controller, are we? */
  uint16_t external_interface_id;  /* tell ident_rsp should place it in which interface, should start at \c
                                      SPN_EXTERNAL_INTERFACE_BASE */
  struct dcp_ctx* dcp_ctx;         /* used to access db when handling timeout callback */
  uint16_t response_delay;         /* timeout in milliseconds */
  enum dcp_state state;

  /** So many filter we should support, just pick essential */
  uint8_t mac_addr[6]; /* BE */
  uint32_t ip_addr;    /* BE */
  uint32_t ip_mask;    /* BE */
  uint32_t ip_gw;      /* BE */
  const char* station_name;
  const char* vendor_name;
  const char* alias_name;
  uint16_t vendor_id; /* LE */
  uint16_t device_id; /* LE */
};

/**
 * Multicast receiver context
 *
 * @note MCR is a stateful context, it is used to track the state of multicast receiver
 * @note this context is used for handle ident.ind & ident.rsp
 *
 * IDLE -> IDENT_IND: receive ident.req and found empty slot
 * IDLE -> IDLE: receive ident.req and found no empty slot
 * IDENT_IND -> IDENT_RSP: time for response, trigger by timer or ident.req(no response delay)
 * IDENT_RSP -> IDLE: response sent
 * IDENT_IND -> IDLE: filter in ident.req not match or invalid ident.req
 *
 */
struct dcp_mcr_ctx {
  uint32_t xid;                    /* session token */
  uint32_t req_options_bitmap;     /* set bits if option is requested */
  uint16_t response_delay_factory; /* set 1 if we are controller, are we? */
  uint16_t response_delay;         /* timeout in milliseconds, set 0 if no response delay */
  struct eth_addr src_addr;        /* requester's address of ident.req */
  enum dcp_state state;
  struct dcp_ctx* dcp_ctx; /* used to access db when handling timeout callback */
};

struct dcp_ctx {
  /* Internal variables used when acting controller or devices */
  struct db_ctx* db;
  struct dcp_mcr_ctx mcr_ctx[SPN_CONF_DCP_MAX_IDENT_RSP_INST];
  struct dcp_mcs_ctx mcs_ctx;
  struct dcp_ucs_ctx ucs_ctx;

  /** Internal physical layer attributes */
  int interface_id;
  int port_num; /* TODO: not used yet */
};

/*@
  @ // ========== Basic Payload Boundary Properties ==========
  @ 
  @ // Property 1: Valid Frame ID location
  @ // Frame ID must be at bytes 0-1
  @ predicate valid_frame_id_location(char* payload, uint16_t length) =
  @   \valid((char*)payload + (0 .. 1)) &&
  @   length >= 2;
  @
  @ // Property 2: Valid DCP Header location
  @ // DCP header starts after Frame ID and optional VLAN tag (4 bytes)
  @ // Header is 10 bytes: service_id, service_type, xid(4), response_delay(2), data_length(2)
  @ predicate valid_dcp_header_location(char* payload, uint16_t length) =
  @   \valid((char*)payload + (2 .. 11)) &&
  @   length >= 12;
  @
  @ // Property 3: Valid DCP data region
  @ // Data starts after Frame ID (2 bytes) and DCP header (10 bytes)
  @ predicate valid_dcp_data_location(char* payload, uint16_t length) =
  @   \valid((char*)payload + (12 .. length - 1)) &&
  @   length >= 14; // minimum: 2 (frame_id) + 10 (header) + 2 (one block minimum)
  @
  @ // Property 4: Minimum frame length for DCP_SET response
  @ // Frame must contain: Frame ID (2) + Header (10) = 12 bytes minimum
  @ predicate minimum_dcp_frame_length(uint16_t length) =
  @   length >= 12;
  @
  @ // ========== DCP Header Properties ==========
  @
  @ // Property 5: Valid service ID for response
  @ // Service ID must be one of: DCP_SRV_ID_GET (0x03), SET (0x04), IDENT (0x05), HELLO (0x06)
  @ predicate valid_dcp_service_id(uint8_t service_id) =
  @   service_id == (unsigned char)0x03 || service_id == (unsigned char)0x04 || 
  @   service_id == (unsigned char)0x05 || service_id == (unsigned char)0x06;
  @
  @ // Property 6: Valid service type for response
  @ // Service type must be: RES (0x01) or ERR (0x05)
  @ predicate valid_dcp_response_type(uint8_t service_type) =
  @   service_type == (unsigned char)0x01 || service_type == (unsigned char)0x05;
  @
  @ // Property 7: Data length is bounded by payload
  @ // The data_length field (at bytes 10-11) must not exceed remaining payload
  @ // Maximum data length = total_length - 12 (frame_id + header)
  @ predicate valid_data_length_bound(char* payload, uint16_t total_length, uint16_t data_length) =
  @   data_length <= total_length - 12;
  @
  @ // ========== DCP Block Format Properties ==========
  @
  @ // Property 8: Valid block header at offset
  @ // Each block has: option (1), sub_option (1), length (2), data (variable)
  @ predicate valid_block_header_at(char* payload, uint16_t total_length, uint16_t block_offset) =
  @   \valid((char*)payload + (block_offset .. block_offset + 3)) &&
  @   block_offset + 4 <= total_length;
  @
  @ // Property 9: Block data is within payload bounds
  @ // Given block at offset with block_length field, data must not exceed payload
  @ predicate valid_block_data_bounds(char* payload, uint16_t total_length, 
  @                                   uint16_t block_offset, uint16_t block_length) =
  @   block_offset + 4 + block_length <= total_length;
  @
  @ // Property 10: Valid DCP option
  @ // Option must be one of the defined DCP options: IP(0x01), DEV_PROP(0x02), 
  @ // DHCP(0x03), CONTROL(0x05), DEV_INITIATIVE(0x06), NME_DOMAIN(0x07), ALL(0xFF)
  @ predicate valid_dcp_option(uint8_t option) =
  @   option == (unsigned char)0x01 || option == (unsigned char)0x02 || option == (unsigned char)0x03 ||
  @   option == (unsigned char)0x05 || option == (unsigned char)0x06 || option == (unsigned char)0x07 || option == (unsigned char)0xFF;
  @
  @ // ========== DCP_SET Response Specific Properties ==========
  @
  @ // Property 11: DCP_SET response has correct service ID
  @ predicate is_dcp_set_response(char* payload) =
  @   *((uint8_t*)payload + 2) == (unsigned char)0x04; // service_id = DCP_SRV_ID_SET
  @
  @ // Property 12: Block in SET response is CONTROL block
  @ // All blocks in SET response must be CONTROL blocks (option = 0x05)
  @ // with sub_option = RESPONSE (0x04)
  @ predicate is_control_response_block(uint8_t option, uint8_t sub_option) =
  @   option == (unsigned char)0x05 && sub_option == (unsigned char)0x04;
  @
  @ // Property 13: Control response block has minimal data
  @ // Control response block contains: option(1), sub_option(1), error(1), padding(1) = 4 bytes
  @ predicate valid_control_response_block_length(uint16_t block_length) =
  @   block_length == 4 || block_length == 3;
  @
  @ // ========== Message Parsing Properties ==========
  @
  @ // Property 14: All blocks in payload are parseable
  @ // Starting from offset 12 (after frame_id + header), each block can be parsed sequentially
  @ predicate all_blocks_parseable(char* payload, uint16_t total_length) =
  @   valid_dcp_data_location(payload, total_length);
  @
  @ // Property 15: Message can be parsed by dcp_srv_ident_rsp
  @ // Message has valid header, valid blocks, and matches expected format
  @ predicate parseable_by_dcp_srv_ident_rsp(char* payload, uint16_t length,
  @                                          uint8_t service_id, uint8_t service_type) =
  @   valid_frame_id_location(payload, length) &&
  @   valid_dcp_header_location(payload, length) &&
  @   valid_dcp_data_location(payload, length) &&
  @   valid_dcp_service_id(service_id) &&
  @   valid_dcp_response_type(service_type) &&
  @   all_blocks_parseable(payload, length);
  @
  @ // ========== Composed Invariant Properties ==========
  @
  @ // Property 16: Well-formed DCP_SET response message
  @ // Aggregates all properties required for a valid SET response
  @ predicate well_formed_dcp_set_response(char* payload, uint16_t length) =
  @   minimum_dcp_frame_length(length) &&
  @   valid_frame_id_location(payload, length) &&
  @   valid_dcp_header_location(payload, length) &&
  @   is_dcp_set_response(payload) &&
  @   parseable_by_dcp_srv_ident_rsp(payload, length, (unsigned char)0x04, (unsigned char)0x01);
  @
  @ // Property 17: Payload remains valid after SET response construction
  @ // After dcp_srv_set_rsp returns, all message properties are maintained
  @ predicate maintains_message_invariants(char* payload, uint16_t length) =
  @   well_formed_dcp_set_response(payload, length);
  @*/

/*@ axiomatic BuiltinBswap {
  @   logic integer builtin_bswap16_logic(integer x);
  @
  @   axiom bswap16_involutive:
  @     \forall integer x; 0 <= x <= 0xFFFF ==>
  @       builtin_bswap16_logic(builtin_bswap16_logic(x)) == x;
  @
  @   axiom bswap16_range:
  @     \forall integer x; 0 <= x <= 0xFFFF ==>
  @       0 <= builtin_bswap16_logic(x) <= 0xFFFF;
  @ }
  @*/

/*@ logic uint16_t __builtin_bswap16(uint16_t x) =
  @   (uint16_t)builtin_bswap16_logic(x);
  @*/

/*@ assigns \result \from x;
  @ behavior normal:
  @   ensures 0 <= \result <= 0xFFFF;
  @   ensures \result == builtin_bswap16_logic(x);
  @*/
extern uint16_t __builtin_bswap16(uint16_t x);

/*@ axiomatic XidConversion {
  @   logic integer xid_from_header_int(integer xid_high, integer xid_low);
  @
  @   axiom xid_from_header_range:
  @     \forall integer h, l;
  @       0 <= h <= 0xFFFF && 0 <= l <= 0xFFFF ==>
  @       0 <= xid_from_header_int(h, l) <= 0xFFFFFFFF;
  @
  @   axiom xid_from_header_def:
  @     \forall integer h, l;
  @       0 <= h <= 0xFFFF && 0 <= l <= 0xFFFF ==>
  @       xid_from_header_int(h, l) ==
  @       (builtin_bswap16_logic(h) << 16) + builtin_bswap16_logic(l);
  @
  @   axiom xid_from_header_compose:
  @     \forall integer h, l;
  @       0 <= h <= 0xFFFF && 0 <= l <= 0xFFFF ==>
  @       xid_from_header_int(builtin_bswap16_logic(h), builtin_bswap16_logic(l)) ==
  @       (h << 16) + l;
  @
  @   axiom xid_split:
  @     \forall integer xid; 0 <= xid <= 0xFFFFFFFF ==>
  @       xid == ((xid >> 16) << 16) + (xid & 0xFFFF);
  @
  @   axiom xid_compose:
  @     \forall integer xid; 0 <= xid <= 0xFFFFFFFF ==>
  @       (xid & 0xFFFF) + ((xid >> 16) << 16) == xid;
  @
  @   axiom xid_set_matches:
  @     \forall integer xid;
  @       0 <= xid <= 0xFFFFFFFF ==>
  @       xid_from_header_int(
  @         builtin_bswap16_logic(xid >> 16),
  @         builtin_bswap16_logic(xid & 0xFFFF)) == xid;
  @
  @   axiom xid_high_in_range:
  @     \forall integer xid;
  @       0 <= xid <= 0xFFFFFFFF ==>
  @       0 <= (xid >> 16) <= 0xFFFF;
  @
  @   axiom xid_set_matches_uint16:
  @     \forall integer xid;
  @       0 <= xid <= 0xFFFFFFFF ==>
  @       xid_from_header_int(
  @         builtin_bswap16_logic((uint16_t)(xid >> 16)),
  @         builtin_bswap16_logic((uint16_t)(xid & 0xFFFF))) == xid;
  @
  @   predicate xid_matches(struct dcp_header* hdr, uint32_t xid) =
  @     xid_from_header_int(hdr->xid_high, hdr->xid_low) == xid;
  @ }
  @*/

/*@ axiomatic AlignmentOperations {
  @   // Basic alignment properties for dcp_block_next
  @   axiom align_even_idempotent:
  @     \forall integer x; 0 <= x ==>
  @       (((x + 1) & ~1) & 1) == 0;
  @
  @   axiom align_even_range:
  @     \forall integer x; 0 <= x <= 0xFFFF ==>
  @       0 <= ((x + 1) & ~1) <= 0x10000;
  @
  @   axiom align_even_ge:
  @     \forall integer x; 0 <= x ==>
  @       ((x + 1) & ~1) >= x;
  @ }
  @*/

/*@ axiomatic BitOperations {
  @   // Bit test and modulo relationship
  @   axiom bit_test_mod2_equiv:
  @     \forall integer x; 0 <= x ==>
  @       ((x & 1) != 0 <==> x % 2 != 0);
  @
  @   axiom bit_test_mod2_zero:
  @     \forall integer x; 0 <= x ==>
  @       ((x & 1) == 0 <==> x % 2 == 0);
  @
  @   // Bit clearing for bitmap operations
  @   axiom bit_clear_smaller:
  @     \forall integer bm, integer i;
  @       0 <= i < 32 && (bm & (1 << i)) != 0 ==>
  @       (bm & ~(1 << i)) < bm;
  @
  @   axiom bit_clear_preserves_range:
  @     \forall integer bm, integer i, integer n;
  @       0 <= i < n && 0 <= bm < (1 << n) ==>
  @       0 <= (bm & ~(1 << i)) < (1 << n);
  @ }
  @*/


/**
 * @} end of dcp_internal
 */

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************
 * DCP internal functions
 * @defgroup dcp_internal DCP Internal
 */

/*@
  @ requires \valid_read((char*)block + (0 .. sizeof(*block) + SPN_NTOHS(block->length) + 1 - 1));
  @ requires SPN_NTOHS(block->length) >= 0 && SPN_NTOHS(block->length) <= 1500;
  @ assigns \nothing;
  @ ensures \result == ((sizeof(*block) + SPN_NTOHS(block->length) + 1) & ~1);
  @*/

/**
 * @brief walk to next block
 * @param block current block
 * @return offset from current block to next block
 */
int dcp_block_next(struct dcp_block_hdr* block);

/*@
  @ requires \valid(block);
  @ requires \valid(block->data + (0 .. SPN_NTOHS(block->length) - 1));
  @ assigns block->data[SPN_NTOHS(block->length)];
  @ ensures SPN_NTOHS(block->length) % 2 == 0 || block->data[SPN_NTOHS(block->length)] == 0;
  @*/

/**
 * @brief padding dcp block based on its length(BE)
 * @param block current block
 */
void dcp_block_padding(struct dcp_block_hdr* block);

/*@
  @ requires 0 < length <= 1500;
  @ requires \valid((char*)payload + (0 .. \max(length, SPN_RTC_MINIMAL_FRAME_SIZE) - 1));
  @ assigns ((char*)payload)[length .. SPN_RTC_MINIMAL_FRAME_SIZE - 1];
  @ ensures \result >= length;
  @ ensures \result == SPN_RTC_MINIMAL_FRAME_SIZE || \result == length;
  @ ensures length < SPN_RTC_MINIMAL_FRAME_SIZE ==> \result == SPN_RTC_MINIMAL_FRAME_SIZE;
  @ ensures length >= SPN_RTC_MINIMAL_FRAME_SIZE ==> \result == length;
  @*/

/**
 * @brief padding dcp payload
 * @param payload Link layer payload
 * @param length Link layer payload length
 * @return Link layer payload length (with padding)
 */
uint16_t dcp_padding(void* payload, uint16_t length);

/**
 * @brief get name of option/sub option
 * @note this function is invalid if \c SPN_DEBUG is disabled
 * @return name of option/sub option
 */
const char* dcp_option_name(uint8_t option, uint8_t sub_option);

/*@
  @ requires 0 <= option <= 0xFF;
  @ requires 0 <= sub_option <= 0xFF;
  @ assigns \nothing;
  @ ensures \result == -1 || (0 <= \result < DCP_BIT_IDX_NUM);
  @*/

/**
 * @brief get \c dcp_bitmap_idx of option/sub option
 * @param option option
 * @param sub_option sub option
 * @return \c dcp_bitmap_idx
 */
int dcp_option_bit_idx(uint8_t option, uint8_t sub_option);

/*@
  @ requires 0 <= offset < DCP_BIT_IDX_NUM;
  @ assigns \nothing;
  @ ensures \result == BLOCK_TYPE(\result >> 8, \result & 0xFF);
  @*/

/**
 * @brief get option/sub option from \c dcp_bitmap_idx
 * @param offset \c dcp_bitmap_idx
 * @return BLOCK_TYPE(option, sub_option)
 */
uint16_t dcp_option_from_bit_idx(uint32_t offset);

/*@
  @ requires \valid(hdr);
  @ assigns hdr->xid_high, hdr->xid_low;
  @ ensures xid_matches(hdr, xid);
  @*/

/**
 * @brief set xid to dcp header
 * @param hdr dcp header
 * @param xid xid, LE
 */
void dcp_set_xid(struct dcp_header* hdr, uint32_t xid);

/*@
  @ requires \valid_read((char*)hdr + (0 .. sizeof(struct dcp_header) - 1));
  @ assigns \nothing;
  @ ensures \result == xid_from_header_int(hdr->xid_high, hdr->xid_low);
  @*/

/**
 * @brief get xid from dcp header
 * @param hdr dcp header
 * @return xid, LE
 */
uint32_t dcp_get_xid(struct dcp_header* hdr);

/* this function be opened cause it is used in multiple source file. do not use it in any where */
/*@
  @ requires \valid((struct dcp_ucs_ctx*)arg);
  @ assigns ((struct dcp_ucs_ctx*)arg)->state,
  @         ((struct dcp_ucs_ctx*)arg)->req_options_bitmap,
  @         ((struct dcp_ucs_ctx*)arg)->req_qualifier_bitmap,
  @         ((struct dcp_ucs_ctx*)arg)->xid;
  @ ensures ((struct dcp_ucs_ctx*)arg)->state == DCP_STATE_IDLE;
  @ ensures ((struct dcp_ucs_ctx*)arg)->req_options_bitmap == 0;
  @ ensures ((struct dcp_ucs_ctx*)arg)->req_qualifier_bitmap == 0;
  @*/
void _dcp_srv_set_req_timeout(void* arg);

/*@
  @ requires \valid((struct dcp_ucs_ctx*)arg);
  @ assigns ((struct dcp_ucs_ctx*)arg)->state,
  @         ((struct dcp_ucs_ctx*)arg)->req_options_bitmap,
  @         ((struct dcp_ucs_ctx*)arg)->req_qualifier_bitmap,
  @         ((struct dcp_ucs_ctx*)arg)->xid;
  @ ensures ((struct dcp_ucs_ctx*)arg)->state == DCP_STATE_IDLE;
  @ ensures ((struct dcp_ucs_ctx*)arg)->req_options_bitmap == 0;
  @ ensures ((struct dcp_ucs_ctx*)arg)->req_qualifier_bitmap == 0;
  @*/
void _dcp_srv_get_req_timeout(void* arg);
/**
 * @}
 */

/*********************************************
 * DCP Service
 *
 *
 * | service name | IOD | IOC |
 * | ------------ | --- | --- |
 * | ident.req    |     |  X  |
 * | ident.ins    |  X  |  X  |
 * | ident.res    |  X  |  X  |
 * | ident.cnf    |     |  X  |
 * | set/get.req  |     |  X  |
 * | set/get.ind  |  X  |  X  |
 * | set/get.res  |  X  |  X  |
 * | set/get.cnf  |     |  X  |
 * | hello.req    |  X  |     |
 * | hello.ind    |     |  X  |
 * +--------------------------+
 *
 * @defgroup dcp_service DCP Service
 * @{
 */

/*@
  @ requires \valid(ctx);
  @ requires \valid(mcs_ctx);
  @ requires \valid((char*)payload + (0 .. SPN_RTC_MAXIMAL_FRAME_SIZE - 1));
  @ requires \valid(length);
  @ requires mcs_ctx->req_options_bitmap != 0;
  @ requires mcs_ctx->external_interface_id >= SPN_EXTERNAL_INTERFACE_BASE;
  @ 
  @ behavior idle_state:
  @   assumes mcs_ctx->state == DCP_STATE_IDLE;
  @   assigns ((char*)payload)[0 .. SPN_RTC_MAXIMAL_FRAME_SIZE - 1], 
  @           *length, 
  @           mcs_ctx->state, 
  @           mcs_ctx->response_delay,
  @           mcs_ctx->dcp_ctx;
  @   ensures \result == SPN_OK;
  @   ensures mcs_ctx->state == DCP_STATE_IDENT_REQ;
  @   ensures *length <= SPN_RTC_MAXIMAL_FRAME_SIZE;
  @   ensures mcs_ctx->dcp_ctx == ctx;
  @
  @ behavior busy_state:
  @   assumes mcs_ctx->state != DCP_STATE_IDLE;
  @   assigns \nothing;
  @   ensures \result == -SPN_EBUSY;
  @
  @ complete behaviors;
  @ disjoint behaviors;
  @*/

/**
 * @brief Service handler that triggered by controller to identify new device
 *
 * @param ctx DCP context
 * @param mcs_ctx Multicast sender context, used to track the state of multicast sender
 * @param payload LINK layer payload
 * @param[out] length Length of payload
 * @return \c SPN_OK on success, negative value on error
 * @return \c -SPN_EBUSY if the context is not in idle state
 * @note Syntax: DCP-Header,[NameOfStation]^[AliasName], [OtherFilter]*
 * @note One instance is enough for controller
 * @todo Implement reset of filters
 */
int dcp_srv_ident_req(struct dcp_ctx* ctx, struct dcp_mcs_ctx* mcs_ctx, void* payload, uint16_t* length);

/*@
  @ requires \valid(ctx);
  @ requires \valid(mcr_ctx);
  @ requires \valid_read((char*)payload + (0 .. length - 1));
  @ requires length >= sizeof(struct dcp_header);
  @ requires length <= SPN_RTC_MAXIMAL_FRAME_SIZE;
  @ 
  @ behavior valid_request:
  @   assumes length >= SPN_NTOHS(((struct dcp_header*)payload)->data_length) + sizeof(struct dcp_header);
  @   ensures \result == SPN_OK || \result == -SPN_EAGAIN;
  @   ensures \result == SPN_OK ==> mcr_ctx->state == DCP_STATE_IDENT_RES;
  @   ensures \result == -SPN_EAGAIN ==> mcr_ctx->state == DCP_STATE_IDLE;
  @   assigns mcr_ctx->state, mcr_ctx->xid, mcr_ctx->response_delay_factory, 
  @           mcr_ctx->response_delay, mcr_ctx->req_options_bitmap, mcr_ctx->dcp_ctx;
  @
  @ behavior invalid_length:
  @   assumes length < SPN_NTOHS(((struct dcp_header*)payload)->data_length) + sizeof(struct dcp_header);
  @   assigns \nothing;
  @   ensures \result == -SPN_EBADMSG;
  @
  @ complete behaviors;
  @ disjoint behaviors;
  @*/

/**
 * @brief Service handler that indicated by controller that it wants to identify the device
 *
 * @param ctx DCP context
 * @param mcr_ctx Multicast receiver context, used to track the state of multicast receiver
 * @param payload DCP payload
 * @param length Length of payload
 * @note Syntax: DCP-Header,[NameOfStation]^[AliasName], [OtherFilter]*
 * @note Cuase rsp could be delayed by controller, the instance are limited with \c SPN_CONF_DCP_MAX_IDENT_RSP_INST
 * @todo Implement syntax check (this is not nessary, but it is good to have)
 * @todo More filter need to be supported
 * @returns \c SPN_OK on success, negative value on error
 * @returns \c -SPN_EAGAIN if no response is expected
 * @returns \c -SPN_EBADMSG if the payload is too short
 * @returns \c -SPN_EBUSY if no free MCR context
 */
int dcp_srv_ident_ind(struct dcp_ctx* ctx, struct dcp_mcr_ctx* mcr_ctx, void* payload, uint16_t length);

/*@
  @ requires \valid(ctx);
  @ requires \valid(mcr_ctx);
  @ requires \valid((char*)payload + (0 .. SPN_RTC_MAXIMAL_FRAME_SIZE - 1));
  @ requires \valid(length);
  @ requires mcr_ctx->state == DCP_STATE_IDENT_RES;
  @ 
  @ assigns ((char*)payload)[0 .. SPN_RTC_MAXIMAL_FRAME_SIZE - 1], 
  @         *length, 
  @         mcr_ctx->state;
  @ ensures \result == SPN_OK;
  @ ensures *length <= SPN_RTC_MAXIMAL_FRAME_SIZE;
  @ ensures mcr_ctx->state == DCP_STATE_IDLE;
  @*/

/**
 * @brief Service handler that indicated by device that it wants to identify itself
 *
 * @param ctx DCP context
 * @param mcr_ctx Multicast receiver context, used to track the state of multicast receiver
 * @param[out] payload Link layer payload
 * @param[out] length Length of payload
 * @note This function is called in input context, or timer context
 * @todo Implement the unit test by using \c dcp_srv_ident_cnf
 * @returns \c SPN_OK on success
 *
 */
int dcp_srv_ident_rsp(struct dcp_ctx* ctx, struct dcp_mcr_ctx* mcr_ctx, void* payload, uint16_t* length);

/*@
  @ requires \valid(ctx);
  @ requires \valid(mcs_ctx);
  @ requires \valid_read((char*)payload + (0 .. length - 1));
  @ requires length >= sizeof(struct dcp_header);
  @ requires length <= SPN_RTC_MAXIMAL_FRAME_SIZE;
  @ requires mcs_ctx->external_interface_id >= SPN_EXTERNAL_INTERFACE_BASE;
  @ requires ex_itface_id == \null || \valid(ex_itface_id);
  @ 
  @ behavior valid_response:
  @   assumes length >= SPN_NTOHS(((struct dcp_header*)payload)->data_length) + sizeof(struct dcp_header);
  @   ensures \result == SPN_OK || \result == -SPN_ENXIO || \result == -SPN_ENOMEM;
  @   ensures \result == SPN_OK ==> (ex_itface_id != \null ==> *ex_itface_id >= SPN_EXTERNAL_INTERFACE_BASE);
  @   assigns mcs_ctx->external_interface_id;
  @
  @ behavior invalid_message:
  @   assumes length < SPN_NTOHS(((struct dcp_header*)payload)->data_length) + sizeof(struct dcp_header);
  @   assigns \nothing;
  @   ensures \result == -SPN_EBADMSG;
  @
  @ complete behaviors;
  @ disjoint behaviors;
  @*/

/**
 * @brief Service handler that triggered by device to identify itself
 *
 * @param ctx DCP context
 * @param mcs_ctx Multicast sender context, used to track the state of multicast sender
 * @param payload DCP layer payload
 * @param length Length of payload
 * @param[out] ex_itface_id external interface id that device placed in, just for make sure
 * @note This function is called in input context
 * @todo need a machine to notify the controller that the device is ready
 * @todo see if remove ex_itface_id
 * @returns \c SPN_OK on success
 * @returns \c -SPN_EBADMSG on parsing payload error
 * @returns \c -SPN_ENOMEM on no memory
 */
int dcp_srv_ident_cnf(struct dcp_ctx* ctx,
                      struct dcp_mcs_ctx* mcs_ctx,
                      void* payload,
                      uint16_t length,
                      uint16_t* ex_itface_id);

/*@
  @ requires \valid(ctx);
  @ requires \valid(ucs_ctx);
  @ requires \valid(length);
  @ requires ucs_ctx->ex_ifr >= SPN_EXTERNAL_INTERFACE_BASE;
  @ requires \valid((char*)payload + (0 .. SPN_RTC_MINIMAL_FRAME_SIZE - 1));
  @ requires \separated((char*)payload + (0 .. SPN_RTC_MINIMAL_FRAME_SIZE - 1), length, ucs_ctx);
  @ requires ucs_ctx->req_options_bitmap < (1 << DCP_BIT_IDX_NUM);
  @
  @ // 精确指定修改的内存范围，避免使用 assigns \everything
  @ assigns ((char*)payload)[0 .. SPN_PDU_HDR_SIZE + sizeof(struct dcp_header) + 2 * DCP_BIT_IDX_NUM - 1],
  @         *length, ucs_ctx->state;
  @
  @ ensures ucs_ctx->state == DCP_STATE_GET_REQ;
  @ ensures *length >= SPN_PDU_HDR_SIZE + sizeof(struct dcp_header);
  @ ensures *length <= SPN_RTC_MINIMAL_FRAME_SIZE;
  @ ensures \result == SPN_OK;
  @*/
int dcp_srv_get_req(struct dcp_ctx* ctx, struct dcp_ucs_ctx* ucs_ctx, void* payload, uint16_t* length);
/*@
  @ requires \valid(ctx);
  @ requires \valid(ucr_ctx);
  @ requires \valid_read((char*)payload + (0 .. length - 1));
  @ requires length >= sizeof(struct dcp_header);
  @ requires \valid_read((struct dcp_header*)payload);
  @ assigns *ucr_ctx;
  @ ensures \result == SPN_OK || \result == -SPN_EINVAL;
  @*/
int dcp_srv_get_ind(struct dcp_ctx* ctx, struct dcp_ucr_ctx* ucr_ctx, void* payload, uint16_t length);

/*@
  @ requires \valid(ctx) && \valid(ucr) && \valid(length);
  @ requires \valid((char*)payload + (0 .. SPN_RTC_MINIMAL_FRAME_SIZE - 1));
  @ requires \separated((char*)payload + (0 .. SPN_RTC_MINIMAL_FRAME_SIZE - 1), length, ucr, ctx);
  @ requires \valid(ctx->db);
  @ requires 0 <= ctx->interface_id;
  @
  @ assigns ((char*)payload)[0 .. SPN_RTC_MINIMAL_FRAME_SIZE - 1], *length;
  @
  @ ensures \result == SPN_OK;
  @ ensures *length >= SPN_PDU_HDR_SIZE + sizeof(struct dcp_header);
  @ ensures *length <= SPN_RTC_MINIMAL_FRAME_SIZE;
  @*/
int dcp_srv_get_rsp(struct dcp_ctx* ctx, struct dcp_ucr_ctx* ucr, void* payload, uint16_t* length);

/*@
  @ requires \valid(ctx);
  @ requires \valid(ctx->db);
  @ requires \valid(ucs_ctx);
  @ requires ucs_ctx->ex_ifr >= SPN_EXTERNAL_INTERFACE_BASE;
  @ requires \valid((char*)payload + (0 .. length - 1));
  @ requires length >= SPN_PDU_HDR_SIZE + sizeof(struct dcp_header);
  @ requires length <= SPN_RTC_MINIMAL_FRAME_SIZE;
  @ requires \separated(ctx, ucs_ctx, (char*)payload + (0 .. length - 1));
  @
  @ assigns ucs_ctx->state,
  @         ucs_ctx->req_options_bitmap,
  @         ucs_ctx->req_qualifier_bitmap,
  @         ucs_ctx->xid,
  @         ucs_ctx->resp_errors[0 .. DCP_BIT_IDX_NUM - 1],
  @         ctx->db->interfaces[0 .. SPN_DB_MAX_INTERFACE - 1];
  @
  @ behavior success:
  @   assumes ((struct dcp_header*)payload)->xid_high == (uint16_t)(ucs_ctx->xid >> 16);
  @   assumes ((struct dcp_header*)payload)->xid_low == (uint16_t)(ucs_ctx->xid & 0xFFFF);
  @   ensures \result == SPN_OK;
  @   ensures ucs_ctx->state == DCP_STATE_IDLE;
  @   ensures ucs_ctx->req_options_bitmap == 0;
  @   ensures ucs_ctx->req_qualifier_bitmap == 0;
  @   ensures ucs_ctx->xid == \old(ucs_ctx->xid) + 1;
  @
  @ behavior xid_mismatch:
  @   assumes ((struct dcp_header*)payload)->xid_high != (uint16_t)(ucs_ctx->xid >> 16) ||
  @           ((struct dcp_header*)payload)->xid_low != (uint16_t)(ucs_ctx->xid & 0xFFFF);
  @   ensures \result == -SPN_EAGAIN;
  @   ensures ucs_ctx->state == \old(ucs_ctx->state);
  @   ensures ucs_ctx->xid == \old(ucs_ctx->xid);
  @
  @ complete behaviors;
  @ disjoint behaviors;
  @*/
int dcp_srv_get_cnf(struct dcp_ctx* ctx, struct dcp_ucs_ctx* ucs_ctx, void* payload, uint16_t length);

/*@
  @ requires \valid(ctx);
  @ requires \valid(ucs_ctx);
  @ requires \valid(length);
  @ requires \valid((char*)payload + (0 .. SPN_RTC_MINIMAL_FRAME_SIZE - 1));
  @ requires \separated((char*)payload + (0 .. SPN_RTC_MINIMAL_FRAME_SIZE - 1), length, ucs_ctx);
  @ requires ucs_ctx->req_options_bitmap < (1 << DCP_BIT_IDX_NUM);
  @
  @ assigns ((char*)payload)[0 .. SPN_RTC_MINIMAL_FRAME_SIZE - 1], *length;
  @
  @ ensures \result == SPN_OK;
  @ ensures *length >= SPN_PDU_HDR_SIZE + sizeof(struct dcp_header);
  @ ensures *length <= SPN_RTC_MINIMAL_FRAME_SIZE;
  @*/
int dcp_srv_set_req(struct dcp_ctx* ctx, struct dcp_ucs_ctx* ucs_ctx, void* payload, uint16_t* length);

/*@
  @ requires \valid(ctx);
  @ requires \valid(ctx->db);
  @ requires \valid(ucr_ctx);
  @ requires \valid(ucr_ctx->error + (0 .. DCP_BIT_IDX_NUM - 1));
  @ requires \valid_read((char*)payload + (0 .. length - 1));
  @ requires length >= sizeof(struct dcp_header);
  @ requires length <= SPN_RTC_MAXIMAL_FRAME_SIZE;
  @ requires \valid_read((struct dcp_header*)payload);
  @
  @ assigns ucr_ctx->xid, ucr_ctx->req_options_bitmap, 
  @         ucr_ctx->error[0 .. DCP_BIT_IDX_NUM - 1];
  @
  @ ensures \result == SPN_OK;
  @ ensures ucr_ctx->xid == xid_from_header_int(((struct dcp_header*)payload)->xid_high, 
  @                                              ((struct dcp_header*)payload)->xid_low);
  @*/
int dcp_srv_set_ind(struct dcp_ctx* ctx, struct dcp_ucr_ctx* ucr_ctx, void* payload, uint16_t length);
/*@
  @ requires \valid(ctx);
  @ requires \valid(ucr_ctx);
  @ requires \valid((char*)payload + (0 .. *length - 1));
  @ requires \valid(length);
  @ requires *length >= 14 + 8 * DCP_BIT_IDX_NUM;  // enough space for all possible blocks
  @ requires *length <= 1500; // reasonable upper bound for Ethernet frame
  @ 
  @ // Bitmap constraints
  @ requires ucr_ctx->req_options_bitmap >= 0;
  @ requires ucr_ctx->req_options_bitmap < (1 << DCP_BIT_IDX_NUM);
  @ 
  @ // Error array must be valid
  @ requires \valid(ucr_ctx->error + (0 .. DCP_BIT_IDX_NUM - 1));
  @ 
  @ assigns *length, ((char*)payload)[0 .. *length - 1], 
  @         ucr_ctx->req_options_bitmap;
  @ 
  @ ensures \result == SPN_OK;
  @ ensures \at(*length, Post) >= 14;
  @ ensures \at(*length, Post) <= 1500;
  @ ensures ((struct dcp_header*)payload)->service_type == DCP_SRV_TYPE_RES;
  @ ensures ((struct dcp_header*)payload)->service_id == DCP_SRV_ID_SET;
  @ ensures ucr_ctx->req_options_bitmap == 0;  // all bits processed
  @*/
int dcp_srv_set_rsp(struct dcp_ctx* ctx, struct dcp_ucr_ctx* ucr_ctx, void* payload, uint16_t* length);

/*@
  @ requires \valid(ctx);
  @ requires \valid(ctx->db);
  @ requires \valid(ucs_ctx);
  @ requires ucs_ctx->ex_ifr < SPN_DB_MAX_INTERFACE;
  @ requires \valid(ucs_ctx->resp_errors + (0 .. DCP_BIT_IDX_NUM - 1));
  @ requires \valid_read((char*)payload + (0 .. length - 1));
  @ requires length >= sizeof(struct dcp_header);
  @ requires length <= SPN_RTC_MAXIMAL_FRAME_SIZE;
  @ requires \valid_read((struct dcp_header*)payload);
  @
  @ behavior xid_match:
  @   assumes xid_from_header_int(((struct dcp_header*)payload)->xid_high, 
  @                               ((struct dcp_header*)payload)->xid_low) == ucs_ctx->xid;
  @   assigns ucs_ctx->state, ucs_ctx->xid, 
  @           ucs_ctx->resp_errors[0 .. DCP_BIT_IDX_NUM - 1];
  @   ensures \result == SPN_OK || \result == -SPN_EINVAL;
  @   ensures \result == SPN_OK ==> ucs_ctx->state == DCP_STATE_IDLE;
  @
  @ behavior xid_mismatch:
  @   assumes xid_from_header_int(((struct dcp_header*)payload)->xid_high, 
  @                               ((struct dcp_header*)payload)->xid_low) != ucs_ctx->xid;
  @   assigns \nothing;
  @   ensures \result == -SPN_EAGAIN;
  @
  @ complete behaviors;
  @ disjoint behaviors;
  @*/

/**
 * @brief Service handler that indicated by device that it wants to set itself
 * @param ctx DCP context
 * @param ucs_ctx Unicast sender context, used to track the state of unicast sender
 * @param payload DCP layer payload
 * @param length Length of payload
 * @note This function will update external interfaces' db if device says it is done
 * @returns \c SPN_OK on success
 * @returns \c -SPN_EINVAL on invalid payload(required option not found)
 * @returns \c -SPN_ENIO on invalid payload
 */
int dcp_srv_set_cnf(struct dcp_ctx* ctx, struct dcp_ucs_ctx* ucs_ctx, void* payload, uint16_t length);

int dcp_srv_hello_req(void);
int dcp_srv_hello_ind(void);

/**
 * @} end of dcp_service
 */

/*********************************************
 * DCP API
 * @defgroup dcp_api DCP API
 * @{
 */
void dcp_init(struct dcp_ctx* ctx, struct db_ctx* db);

/*@
  @ requires \valid(ctx);
  @ assigns ctx->db;
  @ ensures ctx->db == \null;
  @*/
void dcp_deinit(struct dcp_ctx* ctx);

/*@
  @ requires \valid(ctx);
  @ requires \valid(ctx->db);
  @ requires \valid(ctx->mcr_ctx + (0 .. SPN_CONF_DCP_MAX_IDENT_RSP_INST - 1));
  @ requires \valid_read(src);
  @ requires \valid_read((char*)pdu + (0 .. length - 1));
  @ requires length >= SPN_PDU_HDR_SIZE + sizeof(struct dcp_header);
  @ requires length <= SPN_RTC_MAXIMAL_FRAME_SIZE;
  @ 
  @ assigns ctx->mcr_ctx[0 .. SPN_CONF_DCP_MAX_IDENT_RSP_INST - 1],
  @         ctx->ucs_ctx, ctx->mcs_ctx;
  @
  @ ensures \result == SPN_OK || \result < 0;
  @*/
int dcp_input(struct dcp_ctx* ctx,
              struct spn_iface* iface,
              const struct eth_addr* dst,
              const struct eth_addr* src,
              void* pdu,
              uint16_t length);

/*@
  @ requires \valid(ctx);
  @ requires \valid(iface);
  @ requires \valid_read(dst);
  @ requires frame != \null;
  @ assigns \nothing;
  @ ensures \result == 0;
  @*/
int dcp_output(struct dcp_ctx* ctx, struct spn_iface* iface, const struct eth_addr* dst, spn_frame_t frame);

/**
 * @} end of dcp_api
 */

#ifdef __cplusplus
}
#endif