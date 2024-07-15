#pragma once

#include <spn/config.h>
#include <spn/db.h>

#ifndef SPN_DCP_MAX_SIZE
#define SPN_DCP_MAX_SIZE 1416
#endif

#ifndef SPN_DCP_MIN_SIZE
#define SPN_DCP_MIN_SIZE 42
#endif

enum dcp_service_id {
    DCP_SRV_ID_GET = 0x03,
    DCP_SRV_ID_SET = 0x04,
    DCP_SRV_ID_IDENT = 0x05,
    DCP_SRV_ID_HELLO = 0x06
};

enum dcp_service_type {
    DCP_SRV_TYPE_REQ = 0x00,
    DCP_SRV_TYPE_RES = 0x01,
    DCP_SRV_TYPE_ERR = 0x05
};

enum dcp_option {
    DCP_OPTION_IP = 0x01,
    DCP_OPTION_DEV_PROP = 0x02,
    DCP_OPTION_DHCP = 0x03,
    DCP_OPTION_CONTROL = 0x05,
    DCP_OPTION_DEV_INITIATIVE = 0x06,
    DCP_OPTION_NME_DOMAIN = 0x07,
    DCP_OPTION_ALL_SELECTOR = 0xFF
};

enum dcp_sub_option_ip {
    DCP_SUB_OPT_IP_MAC = 0x01,
    DCP_SUB_OPT_IP_PARAM = 0x02,
    DCP_SUB_OPT_IP_FULL_SUITE = 0x03
};

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
    DCP_SUB_OPT_CTR_START = 0x01,
    DCP_SUB_OPT_CTR_STOP = 0x02,
    DCP_SUB_OPT_CTR_SIGNAL = 0x03,
    DCP_SUB_OPT_CTR_RESPONSE = 0x04,
    DCP_SUB_OPT_CTRL_FACTORY_RESET = 0x05,
    DCP_SUB_OPT_CTRL_RESET_TO_FACTORY = 0x06
};

enum dcp_sub_option_dev_initiative {
    DCP_SUB_OPT_DEV_INITIATIVE = 0x01
};

enum dcp_sub_option_nme {
    DCP_SUB_OPT_NME_DOMAIN = 0x01,
    DCP_SUB_OPT_NME_PRIORITY = 0x02,
    DCP_SUB_OPT_NME_PARAM_UUID = 0x03,
    DCP_SUB_OPT_NME_NAME = 0x04,
    DCP_SUB_OPT_NME_CIM_INTERFACE = 0x05,
};

enum dcp_sub_option_all_selector {
    DCP_SUB_OPT_ALL_SELECTOR = 0xFF
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
    int interface_id;
    enum dcp_state state;

    struct db_ctx* db;

    uint32_t ind_xid; /* recorded requester's xid, used for response */
    uint16_t ind_delay_factory; /* recorded requester's response delay, used for response */

    uint8_t packet_buf[SPN_DCP_MAX_SIZE];
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
/**
 * @}
 */

/*********************************************
 * DCP Service
 * @defgroup dcp_service DCP Service
 * @{
 */

int dcp_srv_ident_req();
int dcp_srv_ident_ind(struct dcp_ctx* ctx, void* payload, uint16_t length);
int dcp_srv_ident_res(struct dcp_ctx* ctx, void* payload, uint16_t length);
int dcp_srv_ident_cnf();

int dcp_srv_get_req();
int dcp_srv_get_ind();
int dcp_srv_get_res();
int dcp_srv_get_cnf();

int dcp_srv_set_req();
int dcp_srv_set_ind();
int dcp_srv_set_res();
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