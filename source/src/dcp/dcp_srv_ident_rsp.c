#include <spn/db.h>
#include <spn/db_ll.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/pdu.h>
#include <spn/sys.h>
#include <string.h>

#define BLOCK_TYPE(option, sub_option) ((option << 8) | sub_option)
#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)ptr + offset))

const uint16_t mandatory_options[] = {
    BLOCK_TYPE(DCP_OPT_IP, DCP_SUB_OPT_IP_PARAM),
    BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR),
    BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION),
    BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ID),
    BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ROLE),
    BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_OPTIONS),
#if 0 /* optional */
    BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_INSTANCE),
    BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_OEM_DEVICE_ID),
    BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_RSI),
    BLOCK_TYPE(DCP_OPT_DEV_INITIATIVE, DCP_SUB_OPT_DEV_INITIATIVE),
    BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_DOMAIN),
    BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_PRIORITY),
    BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_PARAM_UUID),
    BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_NAME),
    BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_CIM_INTERFACE),
#endif
};

const uint16_t supported_options[] = {
    BLOCK_TYPE(DCP_OPT_IP, DCP_SUB_OPT_IP_MAC),
    BLOCK_TYPE(DCP_OPT_IP, DCP_SUB_OPT_IP_PARAM),
    BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR),
    BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION),
    BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ID),
    BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ROLE),
    BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_OPTIONS),
#if 0
    BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_ALIAS),
    BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_INSTANCE),
    BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_OEM_DEVICE_ID),
    BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_STD_GATEWAY),
    BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_RSI),
#endif
    BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_START),
    BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_STOP),
    BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_SIGNAL),
    BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_RESPONSE),
    BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_FACTORY_RESET),
    BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_RESET_TO_FACTORY),
#if 0
    BLOCK_TYPE(DCP_OPT_DEV_INITIATIVE, DCP_SUB_OPT_DEV_INITIATIVE),
    BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_DOMAIN),
    BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_PRIORITY),
    BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_PARAM_UUID),
    BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_NAME),
    BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_CIM_INTERFACE),
#endif
    BLOCK_TYPE(DCP_OPT_ALL_SELECTOR, DCP_SUB_OPT_ALL_SELECTOR),
};

static int pack_ident_rsp(struct dcp_ctx* ctx, uint16_t option, uint16_t block_info, struct dcp_block_hdr* block) {
  struct db_object* obj;
  int res;
  block->option = option >> 8;
  block->sub_option = option & 0xFF;
  switch (option) {
    case BLOCK_TYPE(DCP_OPT_IP, DCP_SUB_OPT_IP_PARAM): {
      uint32_t ip, mask, gw;
      res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_ADDR, &obj);
      SPN_ASSERT("No ip address", res >= 0);
      ip = obj->data.u32;
      res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_MASK, &obj);
      SPN_ASSERT("No ip mask", res >= 0);
      mask = obj->data.u32;
      res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_GATEWAY, &obj);
      SPN_ASSERT("No ip gateway", res >= 0);
      gw = obj->data.u32;
      *PTR_OFFSET(block->data, 0, uint16_t) = SPN_HTONS(block_info);
      *PTR_OFFSET(block->data, 2, uint32_t) = ip;
      *PTR_OFFSET(block->data, 6, uint32_t) = mask;
      *PTR_OFFSET(block->data, 10, uint32_t) = gw;
      block->length = 14;
      break;
    }
    case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR): {
      res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_NAME_OF_VENDOR, &obj);
      SPN_ASSERT("No vendor name", res >= 0);
      *PTR_OFFSET(block->data, 0, uint16_t) = 0;
      db_strcpy_obj2str((char*)block->data + 2, obj);
      block->length = 2 + db_object_len(obj);
      break;
    }
    case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION): {
      res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_NAME_OF_INTERFACE, &obj);
      SPN_ASSERT("No station name", res >= 0);
      *PTR_OFFSET(block->data, 0, uint16_t) = 0;
      db_strcpy_obj2str((char*)block->data + 2, obj);
      block->length = 2 + db_object_len(obj);
      break;
    }
    case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ID): {
      uint16_t device_id, vendor_id;
      res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_DEVICE_ID, &obj) < 0;
      SPN_ASSERT("No device id", res >= 0);
      device_id = obj->data.u16;
      res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_VENDOR_ID, &obj);
      SPN_ASSERT("No vendor id", res >= 0);
      vendor_id = obj->data.u16;

      *PTR_OFFSET(block->data, 0, uint16_t) = 0;
      *PTR_OFFSET(block->data, 2, uint16_t) = SPN_HTONS(vendor_id);
      *PTR_OFFSET(block->data, 4, uint16_t) = SPN_HTONS(device_id);
      block->length = 6;
      break;
    }
    case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ROLE): {
      res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_DEVICE_ROLE, &obj);
      SPN_ASSERT("No device role", res >= 0);

      *PTR_OFFSET(block->data, 0, uint16_t) = 0;
      *PTR_OFFSET(block->data, 2, uint8_t) = obj->data.u8;
      *PTR_OFFSET(block->data, 3, uint8_t) = 0;
      block->length = 4;
      break;
    }
    case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_OPTIONS): {
      unsigned i;
      *PTR_OFFSET(block->data, 0, uint16_t) = 0;

      for (i = 0; i < ARRAY_SIZE(supported_options); i++) {
        uint16_t opt = supported_options[i];
        *PTR_OFFSET(block->data, 2 + i * 2, uint16_t) = SPN_HTONS(opt);
      }
      block->length = 2 + ARRAY_SIZE(supported_options) * 2;
      break;
    }
    default:
      SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident.rsp: option %s(%02d:%02d) not supported\n",
                    dcp_option_name(option >> 8, option & 0xff), option >> 8, option & 0xFF);
      return 0;
  }

  res = block->length;
  block->length = SPN_HTONS((block->length));
  /* Padding must be zero */
  dcp_block_padding(block);
  return sizeof(*block) + ((res + 1) & ~1);
}

int dcp_srv_ident_rsp(struct dcp_ctx* ctx, struct dcp_mcr_ctx* mcr, void* payload, uint16_t* length) {
  struct dcp_header* hdr;
  struct db_object* obj;
  uint16_t block_info, idx;
  const int offset_hdr = sizeof(*hdr) + SPN_PDU_HDR_SIZE;
  int res, offset = offset_hdr;
  /* get ip block info */
  res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_BLOCK_INFO, &obj);
  SPN_ASSERT("No ip block info", res >= 0);
  block_info = obj->data.u16;

  for (idx = 0; idx < ARRAY_SIZE(mandatory_options); idx++) {
    uint16_t option = mandatory_options[idx];
    struct dcp_block_hdr* block = PTR_OFFSET(payload, offset, struct dcp_block_hdr);

    res = pack_ident_rsp(ctx, option, block_info, block);
    SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident.rsp: option %s(%02d:%02d) res=%d\n",
                  dcp_option_name(option >> 8, option & 0xff), option >> 8, option & 0xFF, res);
    if (res == 0) {
      continue;
    }

    offset += res;
  }

  hdr = PTR_OFFSET(payload, SPN_PDU_HDR_SIZE, struct dcp_header);
  hdr->service_id = DCP_SRV_ID_IDENT;
  hdr->service_type = DCP_SRV_TYPE_RES;
  dcp_set_xid(hdr, mcr->xid);
  hdr->response_delay = 0;
  hdr->data_length = SPN_HTONS(offset - offset_hdr);
  *PTR_OFFSET(payload, 0, uint16_t) = SPN_HTONS(FRAME_ID_DCP_IDENT_RES);

  *length = dcp_padding(payload, offset);

  mcr->state = DCP_STATE_IDLE;
  return SPN_OK;
}
