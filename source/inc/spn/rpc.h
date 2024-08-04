#pragma once

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

#define RPC_FLAG1_LAST_FRAG 0x01
#define RPC_FLAG1_FRAG 0x02
#define RPC_FLAG1_NO_FACK 0x04
#define RPC_FLAG1_MAYBE 0x08
#define RPC_FLAG1_IDEMPOTENT 0x10
#define RPC_FLAG1_BROADCAST 0x20

#define RPC_FLAG2_CANCEL_PENDING 0x01

#pragma pack(push, 1)

typedef union rpc_uuid {
  struct {
    uint32_t data1;
    uint16_t data2;
    uint16_t data3;
    uint8_t data4[8]; /* [0]&[1]: values, see table 335
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

struct rpc_ctx {
  int reserved;
};

#ifdef __cplusplus
extern "C" {
#endif

int rpc_init(struct rpc_ctx* ctx);
int rpc_input(struct rpc_ctx* ctx, void* payload, int length);

int rpc_is_big_endian(struct rpc_hdr* hdr);
void rpc_ntoh(struct rpc_hdr* hdr);
void rpc_hton(struct rpc_hdr* hdr);
void rpc_ndr_ntoh(void* ndr_data, rpc_pkt_type_t type);
void rpc_ndr_hton(void* ndr_data, rpc_pkt_type_t type);

#ifdef __cplusplus
}
#endif