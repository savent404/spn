#include <spn/spn.h>

void dcp_set_xid(struct dcp_header* hdr, uint32_t xid) {
  uint16_t high = (uint16_t)(xid >> 16);
  uint16_t low = (uint16_t)(xid & 0xFFFF);
  
  //@ assert 0 <= high <= 0xFFFF;
  //@ assert 0 <= low <= 0xFFFF;
  
  hdr->xid_high = SPN_HTONS(high);
  hdr->xid_low = SPN_HTONS(low);
  
  //@ assert hdr->xid_high == builtin_bswap16_logic(high);
  //@ assert hdr->xid_low == builtin_bswap16_logic(low);
  //@ assert xid_from_header_int(hdr->xid_high, hdr->xid_low) == xid;
}


uint32_t dcp_get_xid(struct dcp_header* hdr) {
  uint32_t high = SPN_NTOHS(hdr->xid_high);
  uint32_t low = SPN_NTOHS(hdr->xid_low);
  
  //@ assert 0 <= high <= 0xFFFF;
  //@ assert 0 <= low <= 0xFFFF;
  //@ assert high == builtin_bswap16_logic(hdr->xid_high);
  //@ assert low == builtin_bswap16_logic(hdr->xid_low);
  
  uint32_t result = (high << 16) + low;
  return result;
}
