#include <spn/db.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/sys.h>
#include <string.h>

#define BLOCK_TYPE(option, sub_option) ((option << 8) | sub_option)
#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))

int dcp_srv_get_ind(struct dcp_ctx* ctx, struct dcp_ucr_ctx* ucr, void* payload, uint16_t length) {
  struct dcp_header* hdr = (struct dcp_header*)payload;
  unsigned idx, num_option, offset;
  uint8_t option, sub_option;

  SPN_UNUSED_ARG(ctx);
  SPN_UNUSED_ARG(length);

  num_option = SPN_NTOHS(hdr->data_length) / 2;
  if (num_option > DCP_BITMAP_NUM) {
    return -SPN_EINVAL;
  }

  ucr->req_options_bitmap = 0;
  for (idx = 0; idx < num_option; idx++) {
    offset = sizeof(*hdr) + idx * 2;
    option = *PTR_OFFSET(payload, offset, uint8_t);
    sub_option = *PTR_OFFSET(payload, offset + 1, uint8_t);
    ucr->req_options_bitmap |= 1 << dcp_option_bitmap(option, sub_option);
  }

  /* TODO: check option is valid */
  ucr->xid = SPN_NTOHL(hdr->xid);
  return SPN_OK;
}
