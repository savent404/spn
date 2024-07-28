#include <lwip/timeouts.h>
#include <spn/db.h>
#include <spn/errno.h>
#include <spn/spn.h>
#include <string.h>

#define BLOCK_TYPE(option, sub_option) ((option << 8) | sub_option)
#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))

void _dcp_srv_set_req_timeout(void* arg) {
  struct dcp_ucs_ctx* ucs_ctx = (struct dcp_ucs_ctx*)arg;
  SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: set.req timeout!\n");

  ucs_ctx->state = DCP_STATE_IDLE;
  ucs_ctx->req_options_bitmap = 0;
  ucs_ctx->req_qualifier_bitmap = 0;
  ucs_ctx->xid++;
}

int dcp_srv_set_req(struct dcp_ctx* ctx, struct dcp_ucs_ctx* ucs_ctx, void* payload, uint16_t* length) {
  struct dcp_header* hdr = NULL;
  unsigned offset;
  unsigned idx;
  unsigned options = ucs_ctx->req_options_bitmap, qualifer = ucs_ctx->req_qualifier_bitmap;

  offset = sizeof(*hdr) + SPN_PDU_HDR_SIZE;

  for (idx = 0; idx < DCP_BIT_IDX_NUM && options; idx++) {
    uint16_t type;
    uint16_t qual;
    struct dcp_block_hdr* block;

    if ((options & (1 << idx)) == 0) {
      continue;
    }

    type = dcp_option_from_bit_idx(idx);
    qual = qualifer & (1 << idx) ? SPN_HTONS(DCP_QUALIFER_PERSISTENT) : SPN_HTONS(DCP_QUALIFER_TEMP);
    block = PTR_OFFSET(payload, offset, struct dcp_block_hdr);
    options &= ~(1 << idx);

    SPN_UNUSED_ARG(ctx);

    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: set.req: packing type: %s(%04x)...\n", dcp_option_name(type >> 8, type & 0xFF),
                  type);

    switch (type) {
      case BLOCK_TYPE(DCP_OPT_IP, DCP_SUB_OPT_IP_PARAM):
        block->length = 14;
        *PTR_OFFSET(block->data, 2, uint32_t) = ucs_ctx->ip_addr;
        *PTR_OFFSET(block->data, 6, uint32_t) = ucs_ctx->ip_mask;
        *PTR_OFFSET(block->data, 10, uint32_t) = ucs_ctx->ip_gw;
        break;
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION):
        SPN_ASSERT("station name is NULL", ucs_ctx->station_name);
        block->length = 2 + strlen(ucs_ctx->station_name);
        memcpy(PTR_OFFSET(block->data, 2, char), ucs_ctx->station_name, block->length - 2);
        break;
      case BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_START):
      case BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_STOP):
        block->length = 2;
        break;
      case BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_SIGNAL):
        block->length = 4;
        *PTR_OFFSET(block->data, 2, uint16_t) = SPN_PP_HTONS(DCP_SIGNAL_LED_FLASH);
        break;
      case BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_FACTORY_RESET):
        block->length = 2;
        qual = ucs_ctx->factory_reset_qualifer;
        break;
      case BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_RESET_TO_FACTORY):
        block->length = 2;
        qual = ucs_ctx->reset_to_factory_qualifer;
        break;
      default:
        SPN_DEBUG_MSG(SPN_DCP_DEBUG, "Unhandled type: %04x\n", type);
        continue;
    }

    /* first 2 bytes of payload always QualiferBlock */
    block->option = type >> 8;
    block->sub_option = type & 0xff;
    *PTR_OFFSET(block->data, 0, uint16_t) = qual;

    /* set padding to zero if length is odd */
    if (block->length & 1) {
      *PTR_OFFSET(block->data, block->length, uint8_t) = 0;
      block->length++;
    }

    /* Swap length after calculated offset */
    offset += sizeof(*block) + block->length;
    block->length = SPN_HTONS(block->length);
  }

  hdr = PTR_OFFSET(payload, SPN_PDU_HDR_SIZE, struct dcp_header);

  if ((offset + sizeof(*hdr)) < SPN_DCP_MIN_SIZE) {
    memset(PTR_OFFSET(hdr, offset + sizeof(*hdr), uint8_t), 0, SPN_DCP_MIN_SIZE - offset - sizeof(*hdr));
    *length = SPN_DCP_MIN_SIZE;
  } else {
    *length = offset + sizeof(*hdr) + SPN_PDU_HDR_SIZE;
  }

  hdr->service_id = DCP_SRV_ID_SET;
  hdr->service_type = DCP_SRV_TYPE_REQ;
  hdr->data_length = SPN_NTOHS(offset);
  hdr->response_delay = 0; /* NOTE: this is reserved, need to be zero */
  dcp_set_xid(hdr, ucs_ctx->xid);

  *PTR_OFFSET(payload, 0, uint16_t) = SPN_HTONS(FRAME_ID_DCP_GET_SET);

  sys_timeout(SPN_DCP_UC_TIMEOUT, _dcp_srv_set_req_timeout, ucs_ctx);

  return SPN_OK;
}
