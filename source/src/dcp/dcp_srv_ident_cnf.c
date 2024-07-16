#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/sys.h>

#include <string.h>

#define BLOCK_TYPE(option, sub_option) ((option) << 8 | (sub_option))
#define PTR_OFFSET(ptr, offset, type) ((type*)((uintptr_t)(ptr) + (offset)))

/**
 * @brief utils function to copy string to db_value_t
 * 
 * @param value pointer to \c db_value_t
 * @param str source string
 * @param len source string length
 * @return string length if > 0, -ENOMEM if malloc failed
 */
static inline int obj_strdup(db_value_t* value, const char* str, size_t len)
{
    if (len > sizeof(value->str) - 1) {
        value->ptr = malloc(len + 1);
        if (!value->ptr) {
            return -SPN_ENOMEM;
        }
        memcpy(value->ptr, str, len);
        ((char*)value->ptr)[len] = '\0';
    } else {
        memcpy(value->str, str, len);
        value->str[len] = '\0';
    }
    return len;
}

static inline int is_static_str(int length)
{
    db_value_t* v;
    return (unsigned)length < sizeof(v->str);
}

int dcp_srv_ident_cnf(struct dcp_ctx* ctx, void* payload, uint16_t length)
{
    struct dcp_header* hdr = (struct dcp_header*)payload;
    uint16_t dcp_length = SPN_NTOHS(hdr->data_length), block_length;
    struct dcp_block_gen* block;
    db_value_t data;
    unsigned offset;
    struct db_interface interface, *intf;
    int res;
    unsigned i;

    if (length < dcp_length + sizeof(*hdr)) {
        SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_cnf: invalid length\n");
        return -SPN_EBADMSG;
    }

    if (ctx->cnf_xid != SPN_NTOHL(hdr->xid)) {
        SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_cnf: invalid xid\n");
        return -SPN_ENXIO;
    }

    /* Reset temp interface */
    memset(&interface, 0, sizeof(interface));

    for (offset = sizeof(*hdr); offset < dcp_length; offset += dcp_block_next(block)) {
        block = PTR_OFFSET(payload, offset, struct dcp_block_gen);
        SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_cnf: handling block %s(%02x:%02x)\n",
            dcp_option_name(block->option, block->sub_option), block->option, block->sub_option);
        if (offset > length) {
            SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_cnf: invalid block length\n");
            res = -SPN_EBADMSG;
            goto invalid_ret;
        }
        switch (BLOCK_TYPE(block->option, block->sub_option)) {
        case BLOCK_TYPE(DCP_OPTION_IP, DCP_SUB_OPT_IP_PARAM):
            SPN_ASSERT("Invalid block length", SPN_NTOHS(block->length) == 14);

            data.u16 = SPN_NTOHS(*(PTR_OFFSET(block->data, 0, uint16_t)));
            res = db_add_object(&interface.objects, DB_ID_IP_BLOCK_INFO, 0, 0, 2, &data);
            if (res != SPN_OK) {
                goto invalid_ret;
            }

            data.u32 = SPN_NTOHL(*(PTR_OFFSET(block->data, 2, uint32_t)));
            res = db_add_object(&interface.objects, DB_ID_IP_ADDR, 0, 0, 4, &data);
            if (res != SPN_OK) {
                goto invalid_ret;
            }

            data.u32 = SPN_NTOHL(*(PTR_OFFSET(block->data, 6, uint32_t)));
            res = db_add_object(&interface.objects, DB_ID_IP_MASK, 0, 0, 4, &data);
            if (res != SPN_OK) {
                goto invalid_ret;
            }

            data.u32 = SPN_NTOHL(*(PTR_OFFSET(block->data, 10, uint32_t)));
            res = db_add_object(&interface.objects, DB_ID_IP_GATEWAY, 0, 0, 4, &data);
            if (res != SPN_OK) {
                goto invalid_ret;
            }
            break;
        case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR):
        case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION):
        case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_ALIAS): {
            static const db_id_t ids[] = { DB_ID_NAME_OF_VENDOR, DB_ID_NAME_OF_STATION, DB_ID_NAME_OF_STATION };
            static const uint8_t sub_options[] = { DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION, DCP_SUB_OPT_DEV_PROP_NAME_OF_ALIAS };
            block_length = SPN_NTOHS(block->length);
            SPN_ASSERT("Invalid block length", block_length > 2 && block_length < 256);
            res = obj_strdup(&data, PTR_OFFSET(block->data, 2, char), block_length - 2);
            if (res < 0) {
                goto invalid_ret;
            }
            for (i = 0; i < ARRAY_SIZE(sub_options); i++) {
                if (sub_options[i] == block->sub_option) {
                    break;
                }
            }
            res = db_add_object(&interface.objects, ids[i], is_static_str(res) ? 0 : 1, 1, block_length, &data);
            if (res != SPN_OK) {
                goto invalid_ret;
            }
            break;
        }
        case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ID):
            SPN_ASSERT("Invalid block length", SPN_NTOHS(block->length) == 6);
            data.u16 = SPN_NTOHS(*(PTR_OFFSET(block->data, 2, uint16_t)));
            res = db_add_object(&interface.objects, DB_ID_VENDOR_ID, 0, 0, 2, &data);
            if (res != SPN_OK) {
                goto invalid_ret;
            }

            data.u16 = SPN_NTOHS(*(PTR_OFFSET(block->data, 4, uint16_t)));
            res = db_add_object(&interface.objects, DB_ID_DEVICE_ID, 0, 0, 2, &data);
            if (res != SPN_OK) {
                goto invalid_ret;
            }
            break;
        case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ROLE):
            SPN_ASSERT("Invalid block length", SPN_NTOHS(block->length) == 4);
            data.u8 = *(PTR_OFFSET(block->data, 2, uint8_t));
            res = db_add_object(&interface.objects, DB_ID_DEVICE_ROLE, 0, 0, 1, &data);
            if (res != SPN_OK) {
                goto invalid_ret;
            }
            break;
        case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_INSTANCE):
            SPN_ASSERT("Invalid block length", SPN_NTOHS(block->length) == 4);
            data.u16 = SPN_NTOHS(*(PTR_OFFSET(block->data, 2, uint16_t)));
            res = db_add_object(&interface.objects, DB_ID_DEVICE_INSTANCE, 0, 0, 2, &data);
            if (res != SPN_OK) {
                goto invalid_ret;
            }
            break;
        case BLOCK_TYPE(DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_OPTIONS):
            block_length = SPN_NTOHS(block->length);
            SPN_ASSERT("Invalid block length", block_length > 10);
            /* option number is more than 4 (10bytes), so we need dynamic method to add object */
            data.ptr = malloc(block_length - 2);
            if (!data.ptr) {
                res = -SPN_ENOMEM;
                goto invalid_ret;
            }
            for (i = 2; i < block_length; i += 2) {
                *PTR_OFFSET(data.ptr, i - 2, uint16_t) = SPN_NTOHS(*(PTR_OFFSET(block->data, i, uint16_t)));
            }

            res = db_add_object(&interface.objects, DB_ID_DEVICE_OPTIONS, 1, 1, block_length - 2, &data);
            if (res != SPN_OK) {
                free(data.ptr);
                goto invalid_ret;
            }
            break;
        default:
            SPN_DEBUG_MSG(SPN_DCP_DEBUG, "DCP: ident_cnf: unhandled option %04x\n", BLOCK_TYPE(block->option, block->sub_option));
            break;
        }
    }

    /* Find empty interface and assigned new interface_id */
    res = db_add_interface(ctx->db, ctx->cnf_interface_id);
    if (res < 0) {
        goto invalid_ret;
    }

    res = db_get_interface(ctx->db, ctx->cnf_interface_id, &intf);
    if (res < 0 || !intf) {
        goto invalid_ret;
    }
    intf->flags.is_outside = 1;

    res = db_dup_interface(intf, &interface);
    if (res != SPN_OK) {
        res = -SPN_ENOMEM;
        goto cleanup_interface;
    }

    ctx->cnf_interface_id++;
    db_del_interface(&interface);
    return SPN_OK;
cleanup_interface:
    db_del_interface(intf);
invalid_ret:
    db_del_interface(&interface);
    return res;
}
