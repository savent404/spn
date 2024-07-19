#include <spn/db.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/pdu.h>
#include <spn/sys.h>
#include <string.h>

#define BLOCK_TYPE(option, sub_option) ((option << 8) | sub_option)
#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)ptr + offset))

const uint16_t mandatory_options[] = {
    BLOCK_TYPE(DCP_OPTION_IP, DCP_SUB_OPT_IP_PARAM),
    BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR),
    BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION),
    BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ID),
    BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ROLE),
    BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_OPTIONS),
#if 0 /* optional */
    BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_INSTANCE),
    BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_OEM_DEVICE_ID),
    BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_RSI),
    BLOCK_TYPE(DCP_OPTION_DEV_INITIATIVE, DCP_SUB_OPT_DEV_INITIATIVE),
    BLOCK_TYPE(DCP_OPTION_NME_DOMAIN, DCP_SUB_OPT_NME_DOMAIN),
    BLOCK_TYPE(DCP_OPTION_NME_DOMAIN, DCP_SUB_OPT_NME_PRIORITY),
    BLOCK_TYPE(DCP_OPTION_NME_DOMAIN, DCP_SUB_OPT_NME_PARAM_UUID),
    BLOCK_TYPE(DCP_OPTION_NME_DOMAIN, DCP_SUB_OPT_NME_NAME),
    BLOCK_TYPE(DCP_OPTION_NME_DOMAIN, DCP_SUB_OPT_NME_CIM_INTERFACE),
#endif
};

const uint16_t supported_options[] = {
    BLOCK_TYPE(DCP_OPTION_IP, DCP_SUB_OPT_IP_MAC),
    BLOCK_TYPE(DCP_OPTION_IP, DCP_SUB_OPT_IP_PARAM),
    BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR),
    BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION),
    BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ID),
    BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ROLE),
    BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_OPTIONS),
#if 0
    BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_ALIAS),
    BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_INSTANCE),
    BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_OEM_DEVICE_ID),
    BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_STD_GATEWAY),
    BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_RSI),
#endif
    BLOCK_TYPE(DCP_OPTION_CONTROL, DCP_SUB_OPT_CTRL_START),
    BLOCK_TYPE(DCP_OPTION_CONTROL, DCP_SUB_OPT_CTRL_STOP),
    BLOCK_TYPE(DCP_OPTION_CONTROL, DCP_SUB_OPT_CTRL_SIGNAL),
    BLOCK_TYPE(DCP_OPTION_CONTROL, DCP_SUB_OPT_CTRL_RESPONSE),
    BLOCK_TYPE(DCP_OPTION_CONTROL, DCP_SUB_OPT_CTRL_FACTORY_RESET),
    BLOCK_TYPE(DCP_OPTION_CONTROL, DCP_SUB_OPT_CTRL_RESET_TO_FACTORY),
#if 0
    BLOCK_TYPE(DCP_OPTION_DEV_INITIATIVE, DCP_SUB_OPT_DEV_INITIATIVE),
    BLOCK_TYPE(DCP_OPTION_NME_DOMAIN, DCP_SUB_OPT_NME_DOMAIN),
    BLOCK_TYPE(DCP_OPTION_NME_DOMAIN, DCP_SUB_OPT_NME_PRIORITY),
    BLOCK_TYPE(DCP_OPTION_NME_DOMAIN, DCP_SUB_OPT_NME_PARAM_UUID),
    BLOCK_TYPE(DCP_OPTION_NME_DOMAIN, DCP_SUB_OPT_NME_NAME),
    BLOCK_TYPE(DCP_OPTION_NME_DOMAIN, DCP_SUB_OPT_NME_CIM_INTERFACE),
#endif
    BLOCK_TYPE(DCP_OPTION_ALL_SELECTOR, DCP_SUB_OPT_ALL_SELECTOR),
};

static inline int obj_strcpy(void* dst, struct db_object* obj) {
  if (db_is_static_string_object(obj)) {
    memcpy(dst, obj->data.str, obj->header.len);
  } else {
    memcpy((char*)dst + 2, obj->data.ptr, obj->header.len);
  }
  return obj->header.len;
}

int dcp_srv_ident_rsp(struct dcp_ctx* ctx, struct dcp_mcr_ctx* mcr, void* payload, uint16_t length) {
  struct dcp_header* hdr = (struct dcp_header*)payload;
  struct db_object* obj;
  uint16_t block_info, idx;
  int res, offset = sizeof(*hdr);
  /* get ip block info */
  res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_BLOCK_INFO, &obj);
  if (res < 0) {
    goto invalid_option;
  }
  block_info = obj->data.u16;

  for (idx = 0; idx < ARRAY_SIZE(mandatory_options); idx++) {
    uint16_t option = mandatory_options[idx];
    struct dcp_block_gen* block = (struct dcp_block_gen*)((uintptr_t)payload + offset);

    block->option = option >> 8;
    block->sub_option = option & 0xFF;

    switch (option) {
      case BLOCK_TYPE(DCP_OPTION_IP, DCP_SUB_OPT_IP_PARAM): {
        uint32_t ip, mask, gw;
        if ((res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_ADDR, &obj)) < 0) {
          goto invalid_option;
        }
        ip = obj->data.u32;
        if ((res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_MASK, &obj)) < 0) {
          goto invalid_option;
        }
        mask = obj->data.u32;
        if ((res = db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_IP_GATEWAY, &obj)) < 0) {
          goto invalid_option;
        }
        gw = obj->data.u32;
        *PTR_OFFSET(block->data, 0, uint16_t) = SPN_HTONS(block_info);
        *PTR_OFFSET(block->data, 2, uint32_t) = SPN_HTONL(ip);
        *PTR_OFFSET(block->data, 6, uint32_t) = SPN_HTONL(mask);
        *PTR_OFFSET(block->data, 10, uint32_t) = SPN_HTONL(gw);
        block->length = 14;
        break;
      }
      case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR): {
        if (db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_NAME_OF_VENDOR, &obj) < 0) {
          goto invalid_option;
        }
        *PTR_OFFSET(block->data, 0, uint16_t) = 0;
        obj_strcpy((char*)block->data + 2, obj);
        block->length = 2 + obj->header.len;
        break;
      }
      case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION): {
        if (db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_NAME_OF_STATION, &obj) < 0) {
          goto invalid_option;
        }
        *PTR_OFFSET(block, 0, uint16_t) = 0;
        obj_strcpy((char*)block->data + 2, obj);
        block->length = 2 + obj->header.len;
        break;
      }
      case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ID): {
        uint16_t device_id, vendor_id;
        if (db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_DEVICE_ID, &obj) < 0) {
          goto invalid_option;
        }
        device_id = obj->data.u16;
        if (db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_VENDOR_ID, &obj) < 0) {
          goto invalid_option;
        }
        vendor_id = obj->data.u16;

        *PTR_OFFSET(block->data, 0, uint16_t) = 0;
        *PTR_OFFSET(block->data, 2, uint16_t) = SPN_HTONS(vendor_id);
        *PTR_OFFSET(block->data, 4, uint16_t) = SPN_HTONS(device_id);
        block->length = 6;
        break;
      }
      case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ROLE): {
        if (db_get_interface_object(ctx->db, ctx->interface_id, DB_ID_DEVICE_ROLE, &obj) < 0) {
          goto invalid_option;
        }

        *PTR_OFFSET(block->data, 0, uint16_t) = 0;
        *PTR_OFFSET(block->data, 2, uint16_t) = SPN_HTONS(obj->data.u16);
        block->length = 4;
        break;
      }
      case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_OPTIONS): {
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
        goto invalid_option;
    }

    offset = (sizeof(*block) + block->length + 1) & ~1;
    block->length = SPN_HTONS(block->length);

    if (offset > length) {
      return -SPN_EMSGSIZE;
    }
  }
  SPN_ASSERT("So tiny!", offset <= SPN_RTC_MAXIMAL_FRAME_SIZE);
  hdr->service_id = DCP_SRV_ID_IDENT;
  hdr->service_id = DCP_SRV_TYPE_RES;
  hdr->xid = SPN_HTONL(mcr->xid);
  hdr->response_delay = SPN_HTONS(mcr->response_delay_factory);
  hdr->data_length = SPN_HTONS((offset - sizeof(*hdr) + 1) & ~1);

  mcr->state = DCP_STATE_IDLE;
  return offset;
invalid_option:

  mcr->state = DCP_STATE_IDLE;
  return -SPN_EBADMSG;
}
