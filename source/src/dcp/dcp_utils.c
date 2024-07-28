#include <spn/spn.h>

void dcp_set_xid(struct dcp_header* hdr, uint32_t xid) {
  hdr->xid_high = SPN_HTONS(xid >> 16);
  hdr->xid_low = SPN_HTONS(xid & 0xFFFF);
}

uint32_t dcp_get_xid(struct dcp_header* hdr) {
  return (SPN_NTOHS(hdr->xid_high) << 16) | SPN_NTOHS(hdr->xid_low);
}
