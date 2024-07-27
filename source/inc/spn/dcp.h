#pragma once

#include <spn/config.h>
#include <spn/db.h>
#include <spn/pdu.h>

#include <netif/ethernet.h>
#include <spn/iface.h>

#ifndef SPN_DCP_MAX_SIZE
#define SPN_DCP_MAX_SIZE 1416
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

enum dcp_service_id { DCP_SRV_ID_GET = 0x03, DCP_SRV_ID_SET = 0x04, DCP_SRV_ID_IDENT = 0x05, DCP_SRV_ID_HELLO = 0x06 };

enum dcp_service_type { DCP_SRV_TYPE_REQ = 0x00, DCP_SRV_TYPE_RES = 0x01, DCP_SRV_TYPE_ERR = 0x05 };

enum dcp_qualifer_type { DCP_QUALIFER_TEMP = 0x00, DCP_QUALIFER_PERSISTENT = 0x01 };

enum dcp_signal_type { DCP_SIGNAL_LED_FLASH = 0x0004 };

enum dcp_option {
  DCP_OPTION_IP = 0x01,
  DCP_OPTION_DEV_PROP = 0x02,
  DCP_OPTION_DHCP = 0x03,
  DCP_OPTION_CONTROL = 0x05,
  DCP_OPTION_DEV_INITIATIVE = 0x06,
  DCP_OPTION_NME_DOMAIN = 0x07,
  DCP_OPTION_ALL_SELECTOR = 0xFF
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
enum dcp_bitmap {
  DCP_BITMAP_CONTROL_START = 0,
  DCP_BITMAP_ALL_SELECTOR,
  DCP_BITMAP_DEVICE_PROPERTIES_NAME_OF_STATION,
  DCP_BITMAP_DEVICE_PROPERTIES_NAME_OF_ALIAS,
  DCP_BITMAP_IP_MAC_ADDRESS,
  DCP_BITMAP_IP_PARAMETER,
  DCP_BITMAP_IP_FULL_IP_SUITE,
  DCP_BITMAP_DEVICE_PROPERTIES_NAME_OF_VENDOR,
  DCP_BITMAP_DEVICE_PROPERTIES_DEVICE_ID,
  DCP_BITMAP_DEVICE_PROPERTIES_DEVICE_ROLE,
  DCP_BITMAP_DEVICE_PROPERTIES_DEVICE_OPTIONS,
  DCP_BITMAP_DEVICE_PROPERTIES_DEVICE_INSTANCE,
  DCP_BITMAP_DEVICE_PROPERTIES_OEM_DEVICE_ID,
  DCP_BITMAP_DEVICE_PROPERTIES_STANDARD_GATEWAY,
  DCP_BITMAP_DEVICE_PROPERTIES_RSI_PROPERTIES,
  DCP_BITMAP_DHCP_CLIENT_IDENT,
  DCP_BITMAP_CONTROL_SIGNAL,
  DCP_BITMAP_CONTROL_RESPONSE,
  DCP_BITMAP_CONTROL_FACTORY_RESET,
  DCP_BITMAP_CONTROL_RESET_TO_FACTORY,
  DCP_BITMAP_DEVICE_INITIATIVE_DEVICE_INITIATIVE,
  DCP_BITMAP_NME_DOMAIN_NME_DOMAIN,
  DCP_BITMAP_NME_DOMAIN_NME_PRIO,
  DCP_BITMAP_NME_DOMAIN_NME_PARAMETER_UUID,
  DCP_BITMAP_NME_DOMAIN_NME_NAME,
  DCP_BITMAP_NME_DOMAIN_CIM_INTERFACE,
  DCP_BITMAP_CONTROL_STOP,
  DCP_BITMAP_NUM
};

/**
 * Attributes of option/sub option
 *
 * | Sub Option                              | Read | Write | Optional | Filter |
 * |-----------------------------------------|------|-------|----------|--------|
 * | IP_MAC_ADDRESS                          | X    |       |          |        |
 * | IP_PARAMETER                            | X    | X     |          | X      |
 * | IP_FULL_IP_SUITE                        | X    | X     | X        | X      |
 * | DEVICE_PROPERTIES_NAME_OF_VENDOR        | X    |       |          | X      |
 * | DEVICE_PROPERTIES_NAME_OF_STATION       | X    | X     |          | X      |
 * | DEVICE_PROPERTIES_DEVICE_ID             | X    |       |          | X      |
 * | DEVICE_PROPERTIES_DEVICE_ROLE           | X    |       |          | X      |
 * | DEVICE_PROPERTIES_DEVICE_OPTIONS        | X    |       |          | X      |
 * | DEVICE_PROPERTIES_NAME_OF_ALIAS         |      |       |          | X      |
 * | DEVICE_PROPERTIES_DEVICE_INSTANCE       | X    |       |          | X      |
 * | DEVICE_PROPERTIES_OEM_DEVICE_ID         | X    |       |          | X      |
 * | DEVICE_PROPERTIES_STANDARD_GATEWAY      | X    |       |          | X      |
 * | DEVICE_PROPERTIES_RSI_PROPERTIES        | X    |       |          | X      |
 * | DHCP_DHCP                               | X    | X     | X        | X      |
 * | CONTROL_START                           |      | X     |          |        |
 * | CONTROL_STOP                            |      | X     |          |        |
 * | CONTROL_SIGNAL                          |      | X     |          |        |
 * | CONTROL_RESPONSE                        |      |       |          |        |
 * | CONTROL_FACTORY_RESET                   |      |       | X        |        |
 * | CONTROL_RESET_TO_FACTORY                |      | X     |          |        |
 * | DEVICE_INITIATIVE_DEVICE_INITIATIVE     | X    |       |          | X      |
 * | NME_DOMAIN_NME_DOMAIN                   | X    | X     |          | X      |
 * | NME_DOMAIN_NME_PRIO                     | X    |       |          | X      |
 * | NME_DOMAIN_NME_PARAMETER_UUID           | X    |       |          |        |
 * | NME_DOMAIN_NME_NAME                     | X    |       |          | X      |
 * | NME_DOMAIN_CIM_INTERFACE                | X    |       |          | X      |
 * | ALL_SELECTOR                            |      |       |          | X      |
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
  uint32_t xid;
  uint16_t response_delay;
  uint16_t data_length;
};

/**
 * @brief dcp block
 *
 * @note For dcp block, the length is not fixed, it is calculated by the length of data
 * @note As response block, the first word of data is block_info or reserved(if option is not \c DCP_OPTION_IP)
 *
 * | Field          | Size | Description             |
 * |----------------|------|-------------------------|
 * | option         | 1    | Option                  |
 * | sub_option     | 1    | Sub Option              |
 * | length         | 2    | Length of data          |
 * | data           | n    | Data                    |
 * +-------------------------------------------------+
 */
struct dcp_block_gen {
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
 */
struct dcp_ucs_ctx {
  uint32_t xid;
  uint32_t req_options_bitmap;   /* set if option is requested */
  uint16_t req_qualifier_bitmap; /* set if persistent mode is requested */
  enum dcp_block_error resp_errors[DCP_BITMAP_NUM];
  enum dcp_state state;

  /* set if option is requested */
  uint32_t ip_addr;
  uint32_t ip_mask;
  uint32_t ip_gw;
  uint16_t factory_reset_qualifer;
  uint16_t reset_to_factory_qualifer;
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
  uint32_t req_options_bitmap;   /* set if option is requested */
  uint16_t req_qualifier_bitmap; /* set if persistent mode is requested */
  enum dcp_block_error error[DCP_BITMAP_NUM];
};

struct dcp_ctx;

/**
 * Multicast sender context
 *
 * @note MCS is a stateful context, it is used to track the state of multicast sender
 * @note this context is used for handle ident.req & ident.cnf
 *
 * IDEL -> IDENT_REQ: send ident.req
 * IDENT_REQ -> IDENT_REQ: some of device triggered ident.cnf
 * IDENT_REQ -> IDLE: timeout, no response from device should be handled
 */
struct dcp_mcs_ctx {
  uint32_t xid;
  uint32_t req_options_bitmap;
  uint16_t response_delay_factory;
  uint16_t response_delay;
  uint16_t response_interface_id; /* tell ident_rsp should place it in which interface, should start at \c
                                     SPN_EXTERNAL_INTERFACE_BASE */
  struct dcp_ctx* dcp_ctx;
  enum dcp_state state;
  /** So many filter we should support, just pick essential */
  uint8_t mac_addr[6];
  uint32_t ip_addr;
  uint32_t ip_mask;
  uint32_t ip_gw;
  const char* station_name;
  const char* vendor_name;
  const char* alias_name;
  uint16_t vendor_id;
  uint16_t device_id;
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
  uint32_t xid;
  uint32_t req_options_bitmap;
  uint16_t response_delay_factory;
  uint16_t response_delay;
  struct eth_addr src_addr;
  enum dcp_state state;
  struct dcp_ctx* dcp_ctx;
};

struct dcp_ctx {
  /* Internal variables used when acting controller or devices */
  int interface_id;
  int port_num;
  struct db_ctx* db;
  struct dcp_mcr_ctx mcr_ctx[SPN_CONF_DCP_MAX_IDENT_RSP_INST];
  struct dcp_mcs_ctx mcs_ctx;
  struct dcp_ucs_ctx ucs_ctx;

  /** Internal physical layer attributes */
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
int dcp_block_next(struct dcp_block_gen* block);
const char* dcp_option_name(uint8_t option, uint8_t sub_option);
int dcp_option_bitmap(uint8_t option, uint8_t sub_option);
uint16_t dcp_option_bit_offset(uint32_t offset);
void _dcp_srv_set_req_timeout(
    void* arg); /* this function be opened cause it is used in multiple source file. do not use it in any where */
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
 * @note Syntax: DCP-Header,[NameOfStation]^[AliasName], [OtherFilter]*
 * @note One instance is enough for controller
 * @todo Implement reset of filters
 */
int dcp_srv_ident_req(struct dcp_ctx* ctx, struct dcp_mcs_ctx* mcs_ctx, struct pbuf* p);

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
 * @returns 0 on success, negative value on error
 * @returns -SPN_EAGAIN if no response is expected
 * @returns -SPN_EMSGSIZE if the payload is too short
 * @returns -SPN_EBUSY if no free MCR context
 */
int dcp_srv_ident_ind(struct dcp_ctx* ctx, struct dcp_mcr_ctx* mcr_ctx, void* payload, uint16_t length);

/**
 * @brief Service handler that indicated by device that it wants to identify itself
 *
 * @param ctx DCP context
 * @param mcr_ctx Multicast receiver context, used to track the state of multicast receiver
 * @param[out] payload DCP payload
 * @param length Length of payload
 * @note This function is called in input context, or timer context
 * @todo Implement the unit test by using \c dcp_srv_ident_cnf
 * @returns 0 on success, negative value on error
 */
int dcp_srv_ident_rsp(struct dcp_ctx* ctx, struct dcp_mcr_ctx* mcr_ctx, void* payload, uint16_t length);
int dcp_srv_ident_cnf(struct dcp_ctx* ctx,
                      struct dcp_mcs_ctx* mcs_ctx,
                      void* payload,
                      uint16_t length,
                      uint16_t* ex_itface_id);

int dcp_srv_get_req();
int dcp_srv_get_ind(struct dcp_ctx* ctx, struct dcp_ucr_ctx* ucr_ctx, void* payload, uint16_t length);
int dcp_srv_get_rsp(struct dcp_ctx* ctx, struct dcp_ucr_ctx* ucr_ctx, void* payload, uint16_t length);
int dcp_srv_get_cnf();

int dcp_srv_set_req(struct dcp_ctx* ctx, struct dcp_ucs_ctx* ucs_ctx, struct pbuf* p);
int dcp_srv_set_ind(struct dcp_ctx* ctx, struct dcp_ucr_ctx* ucr_ctx, void* payload, uint16_t length);
int dcp_srv_set_rsp(struct dcp_ctx* ctx, struct dcp_ucr_ctx* ucr_ctx, void* payload, uint16_t length);
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
int dcp_input(struct dcp_ctx* ctx, struct spn_iface* iface, const struct eth_addr* src, struct pbuf* p);
int dcp_output(struct dcp_ctx* ctx, struct spn_iface* iface, const struct eth_addr* dst, struct pbuf* p);

/**
 * @} end of dcp_api
 */

#ifdef __cplusplus
}
#endif