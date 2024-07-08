#include <lwip/opt.h>

#include <lwip/arch.h>
#include <lwip/debug.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/pdu.h>

#define IDENT_BLOCK_TYPE(option, sub_option) ((option << 8) | sub_option)

uint16_t spn_dcp_resp_delay(uint16_t rand, uint16_t resp_delay_factor)
{
    uint32_t t = 10 * (rand % resp_delay_factor);
    if (t > SPN_DCP_RESPONSE_DELAY_MAX)
        t = SPN_DCP_RESPONSE_DELAY_MAX;
    return (uint16_t)(t & 0xFFFF);
}

uint16_t spn_dcp_resp_delay_timeout(uint16_t rand, uint16_t resp_delay_factor)
{
    uint32_t t;
    if (resp_delay_factor == SPN_DCP_RESPONSE_DELAY_FACTOR_DEFAULT) {
        return SPN_DCP_RESPONSE_DELAY_DEFAULT;
    }
    t = 1000 + 10 * (rand % resp_delay_factor);
    t = ((t + 999) / 1000) * 1000;
    if (t > SPN_DCP_RESPONSE_DELAY_MAX) {
        t = SPN_DCP_RESPONSE_DELAY_MAX;
    }
    return (uint16_t)(t & 0xFFFF);
}

int spn_dcp_block_parse(struct spn_dcp_header* dcp_hdr, void* payload, uint16_t len, uint16_t offset, struct spn_dcp_identify_block* ident_block)
{
    struct spn_dcp_general_block* gen_block;
    uint16_t block_len;
    uint16_t block_type;

    LWIP_UNUSED_ARG(dcp_hdr);

    gen_block = (struct spn_dcp_general_block*)((uint8_t*)payload + offset);
    block_len = lwip_htons(gen_block->dcp_block_length);
    block_type = IDENT_BLOCK_TYPE(gen_block->option, gen_block->sub_option);

    /* EOF condition */
    if (len <= offset) {
        return SPN_OK;
    }

    if (block_len + sizeof(*gen_block) + offset > len) {
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: invalid block_len=%d, len=%d, offset=%d\n", block_len, len, offset));
        return -SPN_EBADMSG;
    }

    switch (block_type) {
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_MAC_ADDRESS):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing IP.MAC.Address\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_PARAMETER):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing IP.Parameter\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_FULL_IP_SUITE):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing IP.Full.IPSuite\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_VENDER):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.Vender\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_NAME_OF_STATION):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.NameOfStation\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ID):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.DeviceID\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ROLE):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.DeviceRole\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_OPTIONS):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.DeviceOptions\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_ALIAS_NAME):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.AliasName\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_INSTANCE):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.DeviceInstance\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_OEM_DEVICE_ID):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.OEMDeviceID\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_STANDARD_GATEWAY):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.StandardGateway\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_RSI_PROPERTIES):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.RSIProperties\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_DHCP, SPN_DCP_SUB_OPT_DHCP_DHCP):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing DHCP.DHCP\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_CONTROL, SPN_DCP_SUB_OPT_CONTROL_START):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Control.Start\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_CONTROL, SPN_DCP_SUB_OPT_CONTROL_STOP):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Control.Stop\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_CONTROL, SPN_DCP_SUB_OPT_CONTROL_SIGNAL):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Control.Signal\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_CONTROL, SPN_DCP_SUB_OPT_CONTROL_RESPONSE):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Control.Response\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_CONTROL, SPN_DCP_SUB_OPT_CONTROL_FACTORY_RESET):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Control.FactoryReset\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_CONTROL, SPN_DCP_SUB_OPT_CONTROL_RESET_TO_FACTORY):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Control.ResetToFactory\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_INITIATIVE, SPN_DCP_SUB_OPT_DEVICE_INITIATIVE_DEVICE_INITIATIVE):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing DeviceInitiative.DeviceInitiative\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_DOMAIN):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing NMEDomain.NMEDomain\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_PRIO):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing NMEDomain.NMEPrio\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_PARAMETER_UUID):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing NMEDomain.NMEParameterUUID\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_NAME):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing NMEDomain.NMEName\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_CIM_INTERFACE):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing NMEDomain.CIMInterface\n"));
        break;
    case IDENT_BLOCK_TYPE(SPN_DCP_OPTION_ALL_SELECTOR, SPN_DCP_SUB_OPT_ALL_SELECTOR_ALL_SELECTOR):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing AllSelector.AllSelector\n"));
        break;
    default:
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: unknown block_type=0x%04x\n", block_type));
    }

    offset += sizeof(*gen_block) + block_len;
    offset = (offset + 1) & ~1; /* align to 2 bytes */
    return spn_dcp_block_parse(dcp_hdr, payload, len, offset, ident_block);
}

bool spn_dcp_support_multicast(uint8_t service_id, uint8_t service_type)
{
    bool is_req = service_type == SPN_DCP_SERVICE_TYPE_REQUEST;
    return is_req && (service_id == SPN_DCP_SERVICE_ID_IDENTIFY || service_id == SPN_DCP_SERVICE_ID_HELLO);
}

int spn_dcp_input(void* frame, size_t len, uint16_t frame_id, struct eth_hdr* hw_hdr, iface_t* iface)
{
    struct spn_dcp_header* dcp_hdr = (struct spn_dcp_header*)frame;
    struct spn_dcp_general_block* dcp_type = (struct spn_dcp_general_block*)((uint8_t*)dcp_hdr + sizeof(struct spn_dcp_header));
    uint16_t dcp_data_len = PP_HTONS(dcp_hdr->dcp_data_length);
    uint32_t dcp_xid = PP_HTONL(dcp_hdr->xid);
    uint8_t dcp_service_id = dcp_hdr->service_id;
    uint8_t dcp_service_type = dcp_hdr->service_type;

    LWIP_UNUSED_ARG(iface);
    LWIP_UNUSED_ARG(hw_hdr);

    LWIP_DEBUGF(0x80 | LWIP_DBG_TRACE, ("DCP: frame_id=0x%04x, service_id=%d, service_type=%d, xid=0x%08x, dcp_data_len=%d\n", frame_id, dcp_service_id, dcp_service_type, dcp_xid, dcp_data_len));

    /* General check go firstly */
    if (dcp_data_len + sizeof(struct spn_dcp_header) > len || dcp_data_len >= SPN_DCP_DATA_MAX_LENGTH) {
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: invalid dcp_data_len=%d, frame_len=%ld\n", dcp_data_len, len));
        return -SPN_EBADMSG;
    }

    /* check rules related with frame_id and service_id&service_id */
    switch (frame_id) {
    case FRAME_ID_DCP_HELLO_REQ:
        if (dcp_service_id != SPN_DCP_SERVICE_ID_HELLO) {
            goto err_invalid_service_id;
        }
        break;
    case FRAME_ID_DCP_GET_SET:
        if (dcp_service_id != SPN_DCP_SERVICE_ID_GET && dcp_service_id != SPN_DCP_SERVICE_ID_SET) {
            goto err_invalid_service_id;
        }
        break;
    case FRAME_ID_DCP_IDENT_REQ:
        /* Available type:
         * IdentifyAll-Req(NameOfStationBlock^AliasNameBlock^IdentifyReqBlock)
         * IdentifyFilter-Req(AllSelectorBlock) */
        if (dcp_service_id != SPN_DCP_SERVICE_ID_IDENTIFY) {
            goto err_invalid_service_id;
        }
        if (dcp_type->option == SPN_DCP_OPTION_ALL_SELECTOR && dcp_type->sub_option == SPN_DCP_SUB_OPT_ALL_SELECTOR_ALL_SELECTOR) {
            LWIP_ASSERT("dcp_data_len must be 4", dcp_data_len == 4);
        }
        break;
    case FRAME_ID_DCP_IDENT_RES:
        if (dcp_service_id != SPN_DCP_SERVICE_ID_IDENTIFY) {
            goto err_invalid_service_id;
        }
        break;
    default:
        /* TODO: drop unknow frame_id */
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: unknown frame_id=0x%04x\n", frame_id));
        return SPN_OK;
    }

    /* Seems frame's grammar is satisfied, let's do the real job */
    return spn_dcp_block_parse(dcp_hdr, dcp_type, dcp_data_len, 0, NULL);

err_invalid_service_id:
    LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_HALT, ("DCP: invalid service_id=%d\n", dcp_service_id));
    /* TODO: send error response */
    return SPN_OK;
}
