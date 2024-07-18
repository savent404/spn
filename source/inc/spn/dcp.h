#pragma once

#include <spn/config.h>
#include <spn/db.h>

#ifndef SPN_DCP_MAX_SIZE
#define SPN_DCP_MAX_SIZE 1416
#endif

#ifndef SPN_DCP_MIN_SIZE
#define SPN_DCP_MIN_SIZE 42
#endif

enum dcp_service_id { DCP_SRV_ID_GET = 0x03, DCP_SRV_ID_SET = 0x04, DCP_SRV_ID_IDENT = 0x05, DCP_SRV_ID_HELLO = 0x06 };

enum dcp_service_type { DCP_SRV_TYPE_REQ = 0x00, DCP_SRV_TYPE_RES = 0x01, DCP_SRV_TYPE_ERR = 0x05 };

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
  DCP_SUB_OPT_DHCP_UNKNOW = 0xFF,
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

#define DCP_BITMAP_IP_MAC_ADDRESS (0)
#define DCP_BITMAP_IP_PARAMETER (1)
#define DCP_BITMAP_IP_FULL_IP_SUITE (2)
#define DCP_BITMAP_DEVICE_PROPERTIES_NAME_OF_VENDOR (3)
#define DCP_BITMAP_DEVICE_PROPERTIES_NAME_OF_STATION (4)
#define DCP_BITMAP_DEVICE_PROPERTIES_DEVICE_ID (5)
#define DCP_BITMAP_DEVICE_PROPERTIES_DEVICE_ROLE (6)
#define DCP_BITMAP_DEVICE_PROPERTIES_DEVICE_OPTIONS (7)
#define DCP_BITMAP_DEVICE_PROPERTIES_NAME_OF_ALIAS (8)
#define DCP_BITMAP_DEVICE_PROPERTIES_DEVICE_INSTANCE (9)
#define DCP_BITMAP_DEVICE_PROPERTIES_OEM_DEVICE_ID (10)
#define DCP_BITMAP_DEVICE_PROPERTIES_STANDARD_GATEWAY (11)
#define DCP_BITMAP_DEVICE_PROPERTIES_RSI_PROPERTIES (12)
#define DCP_BITMAP_DHCP_DHCP (13)
#define DCP_BITMAP_CONTROL_START (14)
#define DCP_BITMAP_CONTROL_STOP (15)
#define DCP_BITMAP_CONTROL_SIGNAL (16)
#define DCP_BITMAP_CONTROL_RESPONSE (17)
#define DCP_BITMAP_CONTROL_FACTORY_RESET (18)
#define DCP_BITMAP_CONTROL_RESET_TO_FACTORY (19)
#define DCP_BITMAP_DEVICE_INITIATIVE_DEVICE_INITIATIVE (20)
#define DCP_BITMAP_NME_DOMAIN_NME_DOMAIN (21)
#define DCP_BITMAP_NME_DOMAIN_NME_PRIO (22)
#define DCP_BITMAP_NME_DOMAIN_NME_PARAMETER_UUID (23)
#define DCP_BITMAP_NME_DOMAIN_NME_NAME (24)
#define DCP_BITMAP_NME_DOMAIN_CIM_INTERFACE (25)
#define DCP_BITMAP_ALL_SELECTOR (26)

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
  uint8_t data[0];
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
struct dcp_ctx {
  /* Internal variables used when acting controller or devices */
  int interface_id;
  enum dcp_state state;
  struct db_ctx* db;
  uint32_t ind_xid;            /* recorded requester's xid, used for response */
  uint16_t ind_delay_factory;  /* recorded requester's response delay, used for response */
  uint16_t ind_set_req_option; /* Resent set.req's option */
  uint8_t ind_set_req_res;     /* Resent set.req's response */

  /** Internal variables used when acting controller */
  uint32_t cnf_xid;          /* used to filter response that is not belong to this request */
  uint32_t cnf_interface_id; /* auto assigned interface id */
  uint16_t cnf_delay_factory;

  /** Internal variables used when acting device */
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
uint32_t dcp_option_bitmap(uint8_t option, uint8_t sub_option);
uint16_t dcp_option_bit_offset(uint32_t offset);
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

int dcp_srv_ident_req();
int dcp_srv_ident_ind(struct dcp_ctx* ctx, void* payload, uint16_t length);
int dcp_srv_ident_rsp(struct dcp_ctx* ctx, void* payload, uint16_t length);
int dcp_srv_ident_cnf(struct dcp_ctx* ctx, void* payload, uint16_t length);

int dcp_srv_get_req();
int dcp_srv_get_ind();
int dcp_srv_get_rsp();
int dcp_srv_get_cnf();

int dcp_srv_set_req();
int dcp_srv_set_ind(struct dcp_ctx* ctx, void* payload, uint16_t length);
int dcp_srv_set_rsp(struct dcp_ctx* ctx, void* payload, uint16_t length);
int dcp_srv_set_cnf();

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
int dcp_input(struct dcp_ctx* ctx, void* payload, uint16_t length);
int dcp_output(struct dcp_ctx* ctx, void* payload, uint16_t length, uint16_t frame_id);

/**
 * @} end of dcp_api
 */

#ifdef __cplusplus
}
#endif