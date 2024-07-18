#include <spn/dcp.h>
#include <spn/sys.h>

int dcp_block_next(struct dcp_block_gen* block) {
  uint16_t block_length = SPN_NTOHS(block->length);
  uint16_t offset = sizeof(*block) + block_length;
  return (offset + 1) & ~1;
}
