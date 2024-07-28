#include <spn/db.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/pdu.h>
#include <spn/sys.h>
#include <spn/timeout.h>
#include <string.h>

#define BLOCK_TYPE(h, l) ((h << 8) | l)
#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))

static void dcp_srv_ident_req_cb(void* arg) {
  struct dcp_mcs_ctx* mcs = (struct dcp_mcs_ctx*)arg;
  struct dcp_ctx* ctx = mcs->dcp_ctx;

  SPN_ASSERT("dcp_srv_ident_req_cb: invalid mcs", mcs != NULL);
  SPN_ASSERT("dcp_srv_ident_req_cb: invalid ctx", ctx != NULL);

  SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: dcp_srv_ident_req_cb: xid %x\n", mcs->xid);
  /* Devices are already added to db by dcp_srv_ident_cnf */
  mcs->state = DCP_STATE_IDLE;
  mcs->req_options_bitmap = 0;
  mcs->xid++;
}

int dcp_srv_ident_req(struct dcp_ctx* ctx, struct dcp_mcs_ctx* mcs, void* payload, uint16_t* length) {
  int res = SPN_OK;
  unsigned idx, option, offset = 0;
  struct dcp_header* hdr;
  struct dcp_block_hdr* block;
  uint32_t options = mcs->req_options_bitmap;
  const uint32_t required_opt = (1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION) | (1 << DCP_BIT_IDX_ALL_SELECTOR) |
                                (1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_ALIAS);
  const unsigned offset_hdr = SPN_PDU_HDR_SIZE + sizeof(*hdr);

  /* Some mistackes only occurs in our stack */
  SPN_ASSERT("missing required options", (options & required_opt) != 0);
  SPN_ASSERT("invalid length", length != NULL);
  SPN_ASSERT("Invalid interface id going to assign", mcs->external_interface_id >= SPN_EXTERNAL_INTERFACE_BASE);

  if (mcs->state != DCP_STATE_IDLE) {
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident.req: invalid state %d\n", mcs->state);
    return -SPN_EBUSY;
  }

  for (idx = 0; idx < DCP_BIT_IDX_NUM && options; idx++) {
    if (!(options & (1 << idx))) {
      continue;
    }
    options &= ~(1 << idx);
    option = dcp_option_from_bit_idx(idx);
    block = PTR_OFFSET(payload, offset + offset_hdr, struct dcp_block_hdr);
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident.req: option %s(%02d:%02d)\n", dcp_option_name(option >> 8, option & 0xFF),
                  option >> 8, option & 0xFF);
    switch (option) {
      case BLOCK_TYPE(DCP_OPT_ALL_SELECTOR, DCP_SUB_OPT_ALL_SELECTOR):
        block->length = 0;
        break;
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION):
        block->length = strlen(mcs->station_name);
        memcpy(block->data, mcs->station_name, block->length);
        break;
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_ALIAS):
        block->length = strlen(mcs->alias_name);
        memcpy(block->data, mcs->alias_name, block->length);
        break;
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR):
        block->length = strlen(mcs->vendor_name);
        memcpy(block->data, mcs->vendor_name, block->length);
        break;
      case BLOCK_TYPE(DCP_OPT_IP, DCP_SUB_OPT_IP_PARAM):
        block->length = 12;
        *PTR_OFFSET(block->data, 0, uint32_t) = mcs->ip_addr;
        *PTR_OFFSET(block->data, 4, uint32_t) = mcs->ip_mask;
        *PTR_OFFSET(block->data, 8, uint32_t) = mcs->ip_gw;
        break;
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ID):
        block->length = 4;
        *PTR_OFFSET(block->data, 0, uint16_t) = SPN_HTONS(mcs->vendor_id);
        *PTR_OFFSET(block->data, 2, uint32_t) = SPN_HTONS(mcs->device_id);
        break;
      default:
        SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident.req: unknown option %02d:%02d\n", option >> 8, option & 0xFF);
        goto fatal_err;
    }

    /* handle general block header */
    block->option = option >> 8;
    block->sub_option = option & 0xFF;

    /* We need to make sure that the block is 2-byte aligned, and padidng with 0 if not */
    offset += block->length + sizeof(*block);
    if (offset & 1) {
      *PTR_OFFSET(payload, offset + offset_hdr, uint8_t) = 0;
      offset++;
    }
    block->length = SPN_HTONS(block->length);
  }

  /* The payload have to satisfy the minimal frame size */
  if ((offset + offset_hdr) < SPN_RTC_MINIMAL_FRAME_SIZE) {
    memset(PTR_OFFSET(payload, offset + offset_hdr, uint8_t), 0, SPN_RTC_MINIMAL_FRAME_SIZE - offset - offset_hdr);
    *length = SPN_RTC_MINIMAL_FRAME_SIZE;
  } else {
    *length = offset + offset_hdr;
  }

  /* handle general header */
  hdr = PTR_OFFSET(payload, SPN_PDU_HDR_SIZE, struct dcp_header);
  hdr->data_length = SPN_HTONS(offset);
  hdr->response_delay = SPN_HTONS(mcs->response_delay_factory);
  hdr->service_id = DCP_SRV_ID_IDENT;
  hdr->service_type = DCP_SRV_TYPE_REQ;
  dcp_set_xid(hdr, mcs->xid);

  *PTR_OFFSET(payload, 0, uint16_t) = SPN_HTONS(FRAME_ID_DCP_IDENT_REQ);

  /**
   * Response delay is in 10ms units, but the minimum value is 400ms.
   * 400ms if factor < 2
   * round(1s + 10ms * factor) if response_delay_factory >= 2
   */
  mcs->response_delay =
      mcs->response_delay_factory > 1 ? ((1999 + mcs->response_delay_factory * 10) / 1000 * 1000) : 400;
  mcs->dcp_ctx = ctx;
  mcs->state = DCP_STATE_IDENT_REQ;
  mcs->dcp_ctx = ctx;

  SPN_TIMEOUT(mcs->response_delay, dcp_srv_ident_req_cb, mcs);

  return SPN_OK;

fatal_err:
  return res;
}
