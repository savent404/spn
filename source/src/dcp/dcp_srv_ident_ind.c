#include <spn/config.h>
#include <spn/db.h>
#include <spn/db_ll.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/sys.h>
#include <string.h>

#define BLOCK_TYPE(option, sub_option) ((option << 8) | sub_option)
#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))

int dcp_srv_ident_ind(struct dcp_ctx* ctx, struct dcp_mcr_ctx* mcr, void* payload, uint16_t length) {
  struct dcp_header* hdr = (struct dcp_header*)payload;
  struct dcp_block_hdr* block = (struct dcp_block_hdr*)(hdr + 1);
  struct db_object* obj;
  uint32_t options = 0, offset = sizeof(*hdr);
  uint16_t option;
  uint16_t data_len;
  uint16_t mac_k = 0x7843; /* NOTE: initialized value, just in case that device doesn't have its mac address */
  unsigned idx;
  int res;

  if (length < SPN_NTOHS(hdr->data_length) + sizeof(*hdr)) {
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident.ind: payload too short\n");
    return -SPN_EBADMSG;
  }
  length = SPN_NTOHS(hdr->data_length) + sizeof(*hdr);

  /* occupied MCR context from now on */
  mcr->state = DCP_STATE_IDENT_IND;

  for (; offset < length; offset += dcp_block_next(block)) {
    block = PTR_OFFSET(hdr, offset, struct dcp_block_hdr);
    option = BLOCK_TYPE(block->option, block->sub_option);
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident.ind: option %s(%02x:%02x)\n",
                  dcp_option_name(block->option, block->sub_option), block->option, block->sub_option);
    switch (option) {
      case BLOCK_TYPE(DCP_OPT_ALL_SELECTOR, DCP_SUB_OPT_ALL_SELECTOR):
        break;
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION):
        data_len = SPN_NTOHS(block->length);
        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_NAME_OF_INTERFACE, &obj);
        SPN_ASSERT("You must have a name ok?", res == SPN_OK);
        if (data_len != db_object_len(obj) || db_cmp_str2obj(obj, &block->data[0], SPN_NTOHS(block->length)) != 0) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident.ind: name of station mismatch\n");
          goto invalid_req;
        }
        break;
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR):
        data_len = SPN_NTOHS(block->length);
        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_NAME_OF_VENDOR, &obj);
        SPN_ASSERT("Vendor name must be set", res == SPN_OK);
        if (data_len != db_object_len(obj) || db_cmp_str2obj(obj, block->data, SPN_NTOHS(block->length)) != 0) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident.ind: name of vendor mismatch\n");
          goto invalid_req;
        }
        break;
      case BLOCK_TYPE(DCP_OPT_IP, DCP_SUB_OPT_IP_PARAM):
        data_len = SPN_NTOHS(block->length);
        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_ADDR, &obj);
        SPN_ASSERT("IP address must be set", res == SPN_OK);
        if (obj->data.u32 != *PTR_OFFSET(block->data, 0, uint32_t)) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident.ind: IP address mismatch, expected %08x, got %08x\n", obj->data.u32,
                        *PTR_OFFSET(block->data, 0, uint32_t));
          goto invalid_req;
        }

        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_MASK, &obj);
        SPN_ASSERT("IP mask must be set", res == SPN_OK);
        if (obj->data.u32 != *PTR_OFFSET(block->data, 4, uint32_t)) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident.ind: IP mask mismatch, expected %08x, got %08x\n", obj->data.u32,
                        *PTR_OFFSET(block->data, 4, uint32_t));
          goto invalid_req;
        }

        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_GATEWAY, &obj);
        SPN_ASSERT("IP gateway must be set", res == SPN_OK);
        if (obj->data.u32 != *PTR_OFFSET(block->data, 8, uint32_t)) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident.ind: IP gateway mismatch, expected %08x, got %08x\n", obj->data.u32,
                        *PTR_OFFSET(block->data, 8, uint32_t));
          goto invalid_req;
        }
        break;
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ID):
        data_len = SPN_NTOHS(block->length);
        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_VENDOR_ID, &obj);
        SPN_ASSERT("Vendor ID must be set", res == SPN_OK);
        if (SPN_HTONS(obj->data.u16) != *PTR_OFFSET(block->data, 0, uint16_t)) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident.ind: vendor id mismatch, expected %04x, got %04x\n", obj->data.u16,
                        *PTR_OFFSET(block->data, 2, uint16_t));
          goto invalid_req;
        }

        res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_DEVICE_ID, &obj);
        SPN_ASSERT("Device ID must be set", res == SPN_OK);
        if (SPN_HTONS(obj->data.u16) != *PTR_OFFSET(block->data, 2, uint16_t)) {
          SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident.ind: device id mismatch, expected %04x, got %04x\n", obj->data.u16,
                        *PTR_OFFSET(block->data, 0, uint16_t));
          goto invalid_req;
        }
        break;
      case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_ALIAS):
        for (idx = 0; idx < SPN_CONF_MAX_PORT_PER_INTERFACE; idx++) {
          struct db_interface* interface;
          struct db_port* port;
          res = db_get_interface(ctx->db, ctx->interface_id, &interface);
          SPN_ASSERT("interface must be set", res == SPN_OK);
          res = db_get_port(interface, idx, &port);
          if (res != SPN_OK) {
            /* never mind, just skip it */
            continue;
          }
          res = db_get_port_object(ctx->db, ctx->interface_id, idx, DB_ID_NAME_OF_PORT, &obj);
          SPN_ASSERT("port name must be set", res == SPN_OK);
          SPN_ASSERT("port name isn't static str(8)", db_object_len(obj) == 8 && db_is_static_object(obj));

          if (strncmp(obj->data.str, block->data, 8)) {
            continue;
          }

          res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_NAME_OF_INTERFACE, &obj);
          SPN_ASSERT("interface name must be set", res == SPN_OK);

          if (block->data[8] != '.' || SPN_NTOHS(block->length) != db_object_len(obj) + 9 ||
              db_cmp_str2obj(obj, &block->data[9], db_object_len(obj)) != 0) {
            SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident.ind: name of alias mismatch\n");
            goto invalid_req;
          }

          goto alias_matched;
        }
        SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident.ind: name of alias mismatch\n");
        goto invalid_req;
      alias_matched:
        break;
      default:
        SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident.ind: unknown option %d\n", block->option);
        goto invalid_req;
    }
    options |= 1 << dcp_option_bit_idx(block->option, block->sub_option);
  }

  if (offset != length) {
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident.ind: invalid block length\n");
    goto invalid_req;
  }

  /* First option check */
  if (offset == sizeof(*hdr)) {
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident.ind: no option found\n");
    goto invalid_req;
  }

  res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_MAC_ADDR, &obj);
  SPN_ASSERT("MAC address must be set", res == SPN_OK);
  /* NOTE: According to the standard, use the last 2 bytes of MAC address as random number */
  mac_k = obj->data.str[5] | (obj->data.str[4] << 8);

  mcr->xid = dcp_get_xid(hdr);
  mcr->response_delay_factory = SPN_NTOHS(hdr->response_delay);
  mcr->response_delay = 10 * (mac_k % mcr->response_delay_factory);
  mcr->state = DCP_STATE_IDENT_RES;
  mcr->req_options_bitmap = options;
  mcr->dcp_ctx = ctx;
  return SPN_OK;
invalid_req:
  SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident.ind: invalid request, drop it...\n");
  mcr->state = DCP_STATE_IDLE;
  return -SPN_EAGAIN;
}
