#include <spn/dcp.h>
#include <spn/pdu.h>
#include <string.h>

uint16_t dcp_padding(void* payload, uint16_t length) {
  if (length < SPN_RTC_MINIMAL_FRAME_SIZE) {
    memset((char*)payload + length, 0, SPN_RTC_MINIMAL_FRAME_SIZE - length);
    return SPN_RTC_MINIMAL_FRAME_SIZE;
  }
  return length;
}

void dcp_block_padding(struct dcp_block_hdr* block) {
  uint16_t length = SPN_NTOHS(block->length);

  if (length & 1) {
    block->data[length] = 0;
  }
}
