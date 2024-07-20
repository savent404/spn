#include <spn/db.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/pdu.h>
#include <spn/sys.h>
#include <string.h>

#define BLOCK_TYPE(option, sub_option) ((option << 8) | sub_option)
#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))

static uint16_t rsp_block(void* payload, uint16_t option, uint8_t res) {
  struct dcp_block_gen* block_hdr = (struct dcp_block_gen*)payload;
  block_hdr->option = DCP_OPTION_CONTROL;
  block_hdr->sub_option = DCP_SUB_OPT_CTRL_RESPONSE;
  block_hdr->length = SPN_HTONS(3);
  *PTR_OFFSET(payload, sizeof(*block_hdr), uint8_t) = option >> 8;
  *PTR_OFFSET(payload, sizeof(*block_hdr) + 1, uint8_t) = option & 0xFF;
  *PTR_OFFSET(payload, sizeof(*block_hdr) + 2, uint8_t) = res;
  *PTR_OFFSET(payload, sizeof(*block_hdr) + 3, uint8_t) = 0; /* Padding */
  return sizeof(*block_hdr) + 4;
}

int dcp_srv_get_rsp(struct dcp_ctx* ctx, struct dcp_ucr_ctx* ucr, void* payload, uint16_t length) {
  struct dcp_header* hdr = (struct dcp_header*)payload;
  struct db_object* object;
  unsigned idx = 0, offset = sizeof(*hdr);
  unsigned bitmap = ucr->req_options_bitmap;
  int res;

  SPN_UNUSED_ARG(length);

  for (idx = 0; idx < DCP_BITMAP_NUM && bitmap; idx++) {
    uint16_t opt;
    unsigned offset_prev = offset;
    struct dcp_block_gen* block_hdr;
    void* block_data;

    if (!(bitmap & (1 << idx))) {
      continue;
    }
    bitmap &= ~(1 << idx);
    opt = dcp_option_bit_offset(idx);
    block_hdr = PTR_OFFSET(payload, offset, struct dcp_block_gen);
    block_data = PTR_OFFSET(block_hdr, sizeof(*block_hdr), uint8_t);
    switch (opt) {
      case BLOCK_TYPE(DCP_OPTION_IP, DCP_SUB_OPT_IP_PARAM): {
        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_BLOCK_INFO, &object);
        if (res < 0) {
          res = DCP_BLOCK_ERR_RESOURCE_ERR;
          goto rsp_err;
        }
        *PTR_OFFSET(block_data, 0, uint16_t) = SPN_HTONS(object->data.u16);

        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_ADDR, &object);
        if (res < 0) {
          res = DCP_BLOCK_ERR_RESOURCE_ERR;
          goto rsp_err;
        }
        *PTR_OFFSET(block_data, 2, uint32_t) = SPN_HTONL(object->data.u32);

        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_MASK, &object);
        if (res < 0) {
          res = DCP_BLOCK_ERR_RESOURCE_ERR;
          goto rsp_err;
        }
        *PTR_OFFSET(block_data, 6, uint32_t) = SPN_HTONL(object->data.u32);

        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_GATEWAY, &object);
        if (res < 0) {
          res = DCP_BLOCK_ERR_RESOURCE_ERR;
          goto rsp_err;
        }
        *PTR_OFFSET(block_data, 10, uint32_t) = SPN_HTONL(object->data.u32);

        offset += 14 + sizeof(*block_hdr);
      } break;

      default:
        res = DCP_BLOCK_ERR_OPTION_NOT_SUPPORTED;
        goto rsp_err;
    }

    block_hdr->option = opt >> 8;
    block_hdr->sub_option = opt & 0xFF;
    block_hdr->length = SPN_HTONS(offset - offset_prev - sizeof(*block_hdr));
    continue;
  rsp_err:
    offset += rsp_block(block_hdr, opt, res);
  }

  hdr->service_id = DCP_SRV_ID_GET;
  hdr->service_type = DCP_SRV_TYPE_RES;
  hdr->xid = SPN_HTONL(ucr->xid);
  hdr->data_length = SPN_HTONS(offset - sizeof(*hdr));
  hdr->response_delay = 0;

  /* fill zero for padding */
  if (offset < SPN_RTC_MINIMAL_FRAME_SIZE) {
    memset(PTR_OFFSET(payload, offset, uint8_t), 0, SPN_RTC_MINIMAL_FRAME_SIZE - offset);
    offset = SPN_RTC_MINIMAL_FRAME_SIZE;
  }

  return offset;
}
