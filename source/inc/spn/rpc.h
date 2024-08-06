#pragma once

#include <spn/config.h>
#include <stdint.h>

typedef enum rpc_pkt_type {
  RPC_PKT_TYPE_REQ = 0,
  RPC_PKT_TYPE_PING,
  RPC_PKT_TYPE_RESP,
  RPC_PKT_TYPE_FAULT,
  RPC_PKT_TYPE_WORKING,
  RPC_PKT_TYPE_NOCALL, /* Response to ping */
  RPC_PKT_TYPE_REJ,
  RPC_PKT_TYPE_ACK,
  RPC_PKT_TYPE_CON_CHANCEL,
  RPC_PKT_TYPE_FACK, /* Fragment ack */
  RPC_PKT_TYPE_ACK_CHANCEL
} rpc_pkt_type_t;

typedef enum rpc_interface_type {
  RPC_IF_DEVICE = 0,
  RPC_IF_CONTROLLER,
  RPC_IF_SUPERVISOR,
  RPC_IF_PARAM_SERVER,
  RPC_IF_CIM,
  RPC_IF_EPMAP,
} rpc_if_type_t;

/**
 * @brief operation number
 *
 *
 * operation numb for endpoint mapper
 * | val |   Usage    |    Service          |
 * |-----|------------|---------------------|
 * |  0  |  optional  |    Insert           |
 * |  1  |  optional  |    Delete           |
 * |  2  |  Mandatory |    Loopup           |
 * |  3  |  Optional  |    Map              |
 * |  4  |  Mandatory |    LookupHandleFree |
 * |  5  |  Optional  |    InqObject        |
 * |  6  |  Optional  |    MgmtDelete       |
 */
typedef enum rpc_op {
  RPC_OP_CONN = 0,
  RPC_OP_RELEASE,
  RPC_OP_READ,  /* Only valid with ARUUID<>0 */
  RPC_OP_WRITE, /* Only valid with ARUIUID<>0*/
  RPC_OP_CTRL,
  RPC_OP_READ_IMPLICIT, /* Only valid with ARUUID=0 */
  RPC_OP_SEC_CTRL = 10, /* Reserved for security
                         * establishment of a secured communication channel based on RPC or RSI */
} rpc_op_t;

#define RPC_FLAG1_LAST_FRAG 0x02
#define RPC_FLAG1_FRAG 0x04
#define RPC_FLAG1_NO_FACK 0x08
#define RPC_FLAG1_MAYBE 0x10
#define RPC_FLAG1_IDEMPOTENT 0x20
#define RPC_FLAG1_BROADCAST 0x40

#define RPC_FLAG2_CANCEL_PENDING 0x01

#pragma pack(push, 1)

typedef union rpc_uuid {
  struct {
    uint32_t data1;
    uint16_t data2;
    uint16_t data3;
    uint8_t data4[8]; /* [0]&[1]: values, see table 335 (should be fixed 0x8271)
                       * [2]: instance high
                       * [3]: instance low
                       * [4]: deviceId high
                       * [5]: deviceId low
                       * [6]: vendorId high
                       * [7]: vendorId low
                       **/
  } form;
  uint8_t raw[16];
} rpc_uuid_t;

struct rpc_hdr {
  uint8_t version;
  uint8_t packet_type; /* see \c rpc_pkt_type_t */
  uint8_t flag1;       /* see \c RPC_FLAG1_xxx */
  uint8_t flag2;       /* see \c RPC_FLAG2_xxx */
  uint8_t drep1;       /* encoding: bit[0:3]: 1-EBCDIC 0-ASCII; bit[4:7]: 0-big endian 1-little endian */
  uint8_t drep2;       /* Float point representation: 0-IEEE, 1-VAX, 2-CRAY, 3-IBM */
  uint8_t drep3;       /* Should be all zero*/
  uint8_t serial_high;
  rpc_uuid_t object_uuid;
  rpc_uuid_t interface_uuid;
  rpc_uuid_t activity_uuid;
  uint32_t boot_time;
  uint16_t interface_version_major;
  uint16_t interface_version_minor;
  uint32_t seq_numb;
  uint16_t operation_numb; /* provided by PNIO interfaces */
  uint16_t interface_hint; /* 0xFFFF: no hink
                            * the client shall start with 'no hint' for the first call.
                            * and should use the server response for the following calls to allow a server optimization
                            */
  uint16_t activity_hint;  /* Same as interface_hint */
  uint16_t length_of_body; /* number of bytes of NDR Data of the current frame */
  uint16_t frag_numb;      /* number of the current gragment */
  uint8_t auth_protocol;
  uint8_t serial_low;

  char ndr_data[0];
};

struct rpc_ndr_data_req {
  uint32_t args_maxium;
  uint32_t args_length;
  uint32_t maxium_count;
  uint32_t offset; /* Always be zero */
  uint32_t actual_count;
  char pn_pdu[0];
};

#pragma pack(pop)

struct rpc_ctx;

typedef enum rpc_channel_state {
  RPC_CHANNEL_STATE_FREE = 0,
  RPC_CHANNEL_STATE_IDEL,
  /* TODO: Set busy or more details state while handling input pkt */
} rpc_ch_state_t;

struct rpc_channel {
  /* UDP info */
  uint32_t remote_ip;   /* BE */
  uint16_t remote_port; /* LE */
  uint16_t host_port;   /* LE */

  /* RPC info */
  int is_server;
  int is_le;
  enum rpc_op req_op;

  /* Hidden info (user dont care) */
  rpc_uuid_t if_uuid;
  rpc_uuid_t act_uuid;
  rpc_ch_state_t state;
  struct rpc_ctx* ctx;

  char input_buf[SPN_RPC_MAXIUM_BUFFER_SIZE];
  char input_len;

  char output_buf[SPN_RPC_MAXIUM_BUFFER_SIZE];
  char output_len;

  /* Response info */
  enum rpc_pkt_type rsp_pkt_type;
};

typedef void (*rpc_ind_fn_t)(struct rpc_channel* ch, void* payload, unsigned length);
struct rpc_ctx {
  const rpc_uuid_t* act_uuid;
  struct rpc_channel channels[SPN_RPC_MAXIUM_CHANNEL];

  /* NOTE: Use Denpendency Injection to avoid dependency on the upper/lower layer */
  rpc_ind_fn_t fn_rpc_req;
  rpc_ind_fn_t fn_rpc_rsp;
  rpc_ind_fn_t fn_rpc_ping;
  int (*fn_udp_out)(void* payload, int length, uint32_t remote_ip, uint16_t remote_port, uint16_t host_port);
};

#ifdef __cplusplus
extern "C" {
#endif

struct rpc_channel* rpc_channel_alloc(struct rpc_ctx* ctx);
void rpc_channel_free(struct rpc_channel* ch);
struct rpc_channel* rpc_channel_find_by_uuid(struct rpc_ctx* ctx, rpc_uuid_t* uuid);
struct rpc_channel* rpc_channel_find_by_idx(struct rpc_ctx* ctx, int idx);

/**
 * @brief RPC API
 * @defgroup rpc
 */
int rpc_init(struct rpc_ctx* ctx);

/**
 * @brief RPC input API
 * @param ctx RPC context
 * @param payload The payload of the RPC packet
 * @param length The length of the payload
 * @param remote_ip The remote IP address, Big Endian
 * @param remote_port The remote port, Little Endian
 * @param host_port The host port, Little Endian
 *
 * @note The input API should be called by the lower UDP layer or put it in the UDP callback
 * @note This API will allocate a server channel if the act_uuid is not found
 */
int rpc_input(struct rpc_ctx* ctx,
              void* payload,
              int length,
              uint32_t remote_ip,
              uint16_t remote_port,
              uint16_t host_port);

/**
 * @brief RPC allocate a channel acting as a client
 *
 * @param ctx RPC context
 * @param remote_type The remote RPC interface type
 * @param remote_ip The remote IP address, Big Endian
 * @param remote_port The remote port, Little Endian
 * @return The channel index
 *        -SPN_ENOMEM: No free channel
 */
int rpc_get_client_channel(struct rpc_ctx* ctx, rpc_if_type_t remote_type, uint32_t remote_ip, uint16_t remote_port);

/**
 * @brief RPC output API
 * @param ch The RPC channel
 * @param payload The payload of the RPC packet
 * @param length The length of the payload
 *
 * @note Shoule set ch->rsp_pkt_type before calling this API
 * @note The output API should be called by the upper layer
 */
int rpc_output(struct rpc_channel* ch, void* payload, int length);
/**
 * @}
 */

/**
 * @brief Utils API
 * @defgroup rpc_utils
 * @{
 */
int rpc_is_big_endian(struct rpc_hdr* hdr);
void rpc_hdr_ntoh(struct rpc_hdr* hdr);
void rpc_hdr_hton(struct rpc_hdr* hdr);
void rpc_ndr_ntoh(void* ndr_data, rpc_pkt_type_t type);
void rpc_ndr_hton(void* ndr_data, rpc_pkt_type_t type);
/**
 * @brief Get the consistent UUID for the RPC interface
 * @param type The RPC interface type
 * @return The consistent UUID
 */
const rpc_uuid_t* rpc_get_consistent_uuid(enum rpc_interface_type type);
const rpc_uuid_t* rpc_get_object_uuid(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
