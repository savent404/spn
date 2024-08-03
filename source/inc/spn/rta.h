#pragma once

#include <stdint.h>

typedef enum rta_type {
  RTA_TYPE_DATA = 1, /* Version == 1 */
  RTA_TYPE_NACK = 2, /* Version == 1 */
  RTA_TYPE_ACK = 3,
  RTA_TYPE_ERR = 4,
  RTA_TYPE_FREQ = 5, /* Version == 2 */
  RTA_TYPE_FRSP = 6, /* Version == 2 */
} rta_type_t;

typedef enum rta_add_fags {
    win_size_1 = 1,
    win_size_unknow = 1,
    win_size_smallest = 2,
    win_size_optional_u
} rta_add_flags_t;

typedef enum rta_ver {
  RTA_VER_1 = 1, /* FRAME_ID 0xFC01 or 0xFE01 */
  RTA_VER_2 = 2, /* FRAME_ID 0xFE02 */
} rta_ver_t;

typedef union rta_pdu_type {
  struct {
    enum rta_type type : 4;
    enum rta_ver : 4;
  } form;
  uint8_t value;
} rta_pdu_type_t;

typedef struct rta_reference {
  int dst_ap;
  int srv_ap;
} rta_ref_t;

typedef struct rta_rsi_sdu {
  int f_opnum_offset;
  char frags[0]; /* refers to \c rta_si_req_pdu_t or \c rta_rsi_rsp_pdu_t */
} rta_rsi_sdu_t;

typedef struct rta_dpdu {
  rta_pdu_type_t type;
  int flagsSequence;
  int len;
} rta_dpdu_t;

typedef struct rta_rsi_conn_pdu {
  int vendor_id;
  int device_id;
  int instance_id;
  int rsi_interface;
  int padding;
  union {
    int iod_conn_req;
    int iod_read_req;
  } req;
} rta_rsi_conn_pdu;

typedef struct rta_rsi_svcs_pdu {
  int profinet_io_service_req_pdu;
} rta_rsi_svcs_pdu_t;

typedef struct rta_rsi_req_pdu {
  int rsp_max_length;
  union {
    rta_rsi_conn_pdu conn;
    rta_rsi_svcs_pdu_t svcs;
  } pdu;
} rta_rsi_req_pdu_t;

typedef struct rta_rsi_rsp_pdu {
  int pnio_status;
  int others;
} rta_rsi_rsp_pdu_t;

typedef struct rta_data_pdu {
  rta_ref_t ref;
  rta_dpdu_t dpdu; /* type=1, len=1~1432*/
  char sdu[0];
} rta_data_pdu_t;

typedef struct rta_ack_pdu {
  rta_ref_t ref;
  rta_dpdu_t apdu; /* type=3(ack) len=0*/
} rta_ack_pdu_t;

typedef struct rta_nack_pdu {
  rta_ref_t ref;
  rta_dpdu_t npdu; /* type=2(nack) len=0*/
} rta_nack_pdu_t;

typedef struct rta_err_pdu {
  rta_ref_t ref;
  rta_dpdu_t epdu; /* type=4, len=4*/
  int pnio_status;
  int vendor_err_info; /* optional */
} rta_err_pdu_t;

typedef struct rta_freq_pdu {
  rta_ref_t ref;
  rta_dpdu_t fpdu; /* type=5, len=4~1432*/
  char sdu[0];
} rta_freq_pdu_t;

typedef struct rta_frsp_pdu {
  rta_ref_t ref;
  rta_dpdu_t fpdu; /* type=6, len=4~1432*/
  char sdu[0];
} rta_frsp_pdu_t;

typedef struct rta_ao_plain_text {
  uint16_t frame_id;
  union {
    rta_data_pdu_t data;
    rta_ack_pdu_t ack;
    rta_nack_pdu_t nack;
    rta_err_pdu_t err;
    rta_freq_pdu_t freq;
    rta_frsp_pdu_t frsp;
  } rta_pdu;
} rta_ao_plain_text_t;