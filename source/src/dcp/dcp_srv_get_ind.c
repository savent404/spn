#include <spn/db.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/sys.h>
#include <string.h>

#define BLOCK_TYPE(option, sub_option) ((option << 8) | sub_option)
#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))

static inline int has_read_attribute(uint16_t bitmap) {
  const uint32_t read_forbidden = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_ALIAS | 1 << DCP_BIT_IDX_CTRL_START |
                                  1 << DCP_BIT_IDX_CTRL_STOP | 1 << DCP_BIT_IDX_CTRL_SIGNAL |
                                  1 << DCP_BIT_IDX_CTRL_RESPONSE | 1 << DCP_BIT_IDX_CTRL_FACTORY_RESET |
                                  1 << DCP_BIT_IDX_ALL_SELECTOR;
  return (bitmap & read_forbidden) == 0;
}

int dcp_srv_get_ind(struct dcp_ctx* ctx, struct dcp_ucr_ctx* ucr, void* payload, uint16_t length) {
  struct dcp_header* hdr = (struct dcp_header*)payload;
  unsigned idx, num_option, offset;
  int res;
  uint8_t option, sub_option;

  SPN_UNUSED_ARG(ctx);
  SPN_UNUSED_ARG(length);

  num_option = SPN_NTOHS(hdr->data_length) / 2;
  ucr->req_options_bitmap = 0;
  for (idx = 0; idx < num_option; idx++) {
    offset = sizeof(*hdr) + idx * 2;
    option = *PTR_OFFSET(payload, offset, uint8_t);
    sub_option = *PTR_OFFSET(payload, offset + 1, uint8_t);
    res = dcp_option_bitmap(option, sub_option);
    if (res< 0) {
      continue;
    }
    ucr->req_options_bitmap |= 1 << res;
  }

  if (!has_read_attribute(ucr->req_options_bitmap)) {
    return -SPN_EINVAL;
  }

  ucr->xid = dcp_get_xid(hdr);
  return SPN_OK;
}
