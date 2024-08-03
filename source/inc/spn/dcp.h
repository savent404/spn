#pragma once

#include <spn/config.h>
#include <spn/db.h>
#include <spn/pdu.h>

#include <netif/ethernet.h>
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

/**
 * @brief walk to next block
 * @param block current block
 * @return offset from current block to next block
 */
int dcp_block_next(struct dcp_block_hdr* block);

/**
 * @brief padding dcp block based on its length(BE)
 * @param block current block
 */
void dcp_block_padding(struct dcp_block_hdr* block);

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

/**
 * @brief get \c dcp_bitmap_idx of option/sub option
 * @param option option
 * @param sub_option sub option
 * @return \c dcp_bitmap_idx
 */
int dcp_option_bit_idx(uint8_t option, uint8_t sub_option);

/**
 * @brief get option/sub option from \c dcp_bitmap_idx
 * @param offset \c dcp_bitmap_idx
 * @return BLOCK_TYPE(option, sub_option)
 */
uint16_t dcp_option_from_bit_idx(uint32_t offset);

/**
 * @brief set xid to dcp header
 * @param hdr dcp header
 * @param xid xid, LE
 */
void dcp_set_xid(struct dcp_header* hdr, uint32_t xid);

/**
 * @brief get xid from dcp header
 * @param hdr dcp header
 * @return xid, LE
 */
uint32_t dcp_get_xid(struct dcp_header* hdr);

/* this function be opened cause it is used in multiple source file. do not use it in any where */
void _dcp_srv_set_req_timeout(void* arg);
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

int dcp_srv_get_req(struct dcp_ctx* ctx, struct dcp_ucs_ctx* ucs_ctx, void* payload, uint16_t* length);
int dcp_srv_get_ind(struct dcp_ctx* ctx, struct dcp_ucr_ctx* ucr_ctx, void* payload, uint16_t length);
int dcp_srv_get_rsp(struct dcp_ctx* ctx, struct dcp_ucr_ctx* ucr_ctx, void* payload, uint16_t* length);
int dcp_srv_get_cnf(struct dcp_ctx* ctx, struct dcp_ucs_ctx* ucs_ctx, void* payload, uint16_t length);

int dcp_srv_set_req(struct dcp_ctx* ctx, struct dcp_ucs_ctx* ucs_ctx, void* payload, uint16_t* length);
int dcp_srv_set_ind(struct dcp_ctx* ctx, struct dcp_ucr_ctx* ucr_ctx, void* payload, uint16_t length);
int dcp_srv_set_rsp(struct dcp_ctx* ctx, struct dcp_ucr_ctx* ucr_ctx, void* payload, uint16_t* length);

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

int dcp_srv_hello_req();
int dcp_srv_hello_ind();

/**
 * @} end of dcp_service
 */

/*********************************************
 * DCP API
 * @defgroup dcp_api DCP API
 * @{
 */
void dcp_init(struct dcp_ctx* ctx, struct db_ctx* db);
void dcp_deinit(struct dcp_ctx* ctx);
int dcp_input(struct dcp_ctx* ctx,
              struct spn_iface* iface,
              const struct eth_addr* dst,
              const struct eth_addr* src,
              void* pdu,
              uint16_t length);
int dcp_output(struct dcp_ctx* ctx, struct spn_iface* iface, const struct eth_addr* dst, spn_frame_t frame);

/**
 * @} end of dcp_api
 */

#ifdef __cplusplus
}
#endif