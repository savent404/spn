#include <lwip/opt.h>

#include <lwip/arch.h>
#include <lwip/debug.h>
#include <netif/ethernet.h>
#include <lwip/prot/ethernet.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include <spn/pdu.h>
#include <string.h>

#define BLOCK_TYPE(option, sub_option) ((option << 8) | sub_option)
#define GET_VALUE(ptr, type, offset) (*(type*)((uintptr_t)ptr + offset))

/**
 * @brief Find the next block offset
 *
 * @param payload payload points to the first block
 * @param offset payload offset
 * @return int new offset
 */
static inline int spn_dcp_block_walk(void* payload, int offset)
{
    struct spn_dcp_general_block* gen_block;
    uint16_t block_len;
    gen_block = (struct spn_dcp_general_block*)((uint8_t*)payload + offset);
    block_len = lwip_htons(gen_block->dcp_block_length);
    offset += sizeof(*gen_block) + block_len;
    offset = (offset + 1) & ~1; /* align to 2 bytes */
    return offset;
}

const uint16_t dcp_filter_blocks[] = {
    BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_MAC_ADDRESS),
    BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_PARAMETER),
    BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_FULL_IP_SUITE),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_VENDOR),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_NAME_OF_STATION),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ID),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ROLE),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_OPTIONS),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_ALIAS_NAME),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_INSTANCE),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_OEM_DEVICE_ID),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_STANDARD_GATEWAY),
    BLOCK_TYPE(SPN_DCP_OPTION_DHCP, SPN_DCP_SUB_OPT_DHCP_DHCP),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_INITIATIVE, SPN_DCP_SUB_OPT_DEVICE_INITIATIVE_DEVICE_INITIATIVE),
    BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_DOMAIN),
    BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_PRIO),
    BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_NAME),
    BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_CIM_INTERFACE),
    BLOCK_TYPE(SPN_DCP_OPTION_ALL_SELECTOR, SPN_DCP_SUB_OPT_ALL_SELECTOR_ALL_SELECTOR),
};

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

int spn_dcp_block_parse(void* payload, uint16_t len, uint16_t offset, int deep, struct spn_dcp_block* block)
{
    struct spn_dcp_general_block* gen_block;
    uint16_t block_len;
    uint16_t block_type;
    char* r_payload;

    gen_block = (struct spn_dcp_general_block*)((uint8_t*)payload + offset);
    block_len = lwip_htons(gen_block->dcp_block_length);
    block_type = BLOCK_TYPE(gen_block->option, gen_block->sub_option);
    r_payload = (char*)gen_block + sizeof(*gen_block);

    /* EOF condition */
    if (len <= offset) {
        return SPN_OK;
    }

    if (block_len + sizeof(*gen_block) + offset > len) {
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: invalid block_len=%d, len=%d, offset=%d\n", block_len, len, offset));
        return -SPN_EBADMSG;
    }

    if (deep > 32) {
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: too deep\n"));
        return -SPN_EMSGSIZE;
    }

    switch (block_type) {
    case BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_MAC_ADDRESS):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing IP.MAC.Address\n"));
        SPN_DCP_BLOCK_TOUCH(&block->ip_mac);
        LWIP_ASSERT("Invalid block length", block_len == 8);
        block->ip_mac.block_info = lwip_ntohs(GET_VALUE(r_payload, uint16_t, 0));
        memcpy(block->ip_mac.mac_address, r_payload + 2, 6);
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: IP.Mac=%02x:%02x:%02x:%02x:%02x:%02x", block->ip_mac.mac_address[0], block->ip_mac.mac_address[1], block->ip_mac.mac_address[2], block->ip_mac.mac_address[3], block->ip_mac.mac_address[4], block->ip_mac.mac_address[5]));
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_PARAMETER):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing IP.Parameter\n"));
        SPN_DCP_BLOCK_TOUCH(&block->ip_param);
        LWIP_ASSERT("Invalid block length", block_len == 14);
        block->ip_param.block_info = lwip_ntohs(GET_VALUE(r_payload, uint16_t, 0));
        block->ip_param.ip_addr = lwip_ntohl(GET_VALUE(r_payload, uint32_t, 2));
        block->ip_param.mask = lwip_ntohl(GET_VALUE(r_payload, uint32_t, 6));
        block->ip_param.gw = lwip_ntohl(GET_VALUE(r_payload, uint32_t, 10));
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: IP.Parameter=0x%04x, ip=0x%08x, mask=0x%08x, gw=0x%08x\n", block->ip_param.block_info, block->ip_param.ip_addr, block->ip_param.mask, block->ip_param.gw));
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_FULL_IP_SUITE):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing IP.Full.IPSuite\n"));
        SPN_DCP_BLOCK_TOUCH(&block->ip_full_suit);
        LWIP_ASSERT("invalid block length", block_len == 30);
        block->ip_full_suit.block_info = lwip_ntohs(GET_VALUE(r_payload, uint16_t, 0));
        block->ip_full_suit.ip_addr = lwip_ntohl(GET_VALUE(r_payload, uint32_t, 2));
        block->ip_full_suit.mask = lwip_ntohl(GET_VALUE(r_payload, uint32_t, 6));
        block->ip_full_suit.gw = lwip_ntohl(GET_VALUE(r_payload, uint32_t, 10));
        block->ip_full_suit.dns_address[0] = lwip_ntohs(GET_VALUE(r_payload, uint32_t, 14));
        block->ip_full_suit.dns_address[1] = lwip_ntohs(GET_VALUE(r_payload, uint32_t, 18));
        block->ip_full_suit.dns_address[2] = lwip_ntohs(GET_VALUE(r_payload, uint32_t, 22));
        block->ip_full_suit.dns_address[3] = lwip_ntohs(GET_VALUE(r_payload, uint32_t, 26));
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: IP.Parameter=0x%04x, ip=0x%08x, mask=0x%08x, gw=0x%08x, dns[0]=0x%04x, dns[1]=0x%04x, dns[2]=0x%04x, dns[3]=0x%04x\n", block->ip_full_suit.block_info, block->ip_full_suit.ip_addr, block->ip_full_suit.mask, block->ip_full_suit.gw, block->ip_full_suit.dns_address[0], block->ip_full_suit.dns_address[1], block->ip_full_suit.dns_address[2], block->ip_full_suit.dns_address[3]));
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_VENDOR):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.Vendor\n"));
        LWIP_ASSERT("Vendor too long", (int)sizeof(block->dev_prop_vendor.name) > block_len - 3);
        SPN_DCP_BLOCK_TOUCH(&block->dev_prop_vendor);
        strncpy(block->dev_prop_vendor.name, r_payload + 2, block_len - 2);
        block->dev_prop_vendor.name[block_len - 2] = '\0';
        block->dev_prop_vendor.name_len = block_len - 2;
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Vendor=%s\n", block->dev_prop_vendor.name));
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_NAME_OF_STATION):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.NameOfStation\n"));
        LWIP_ASSERT("NameOfStation too long", (int)sizeof(block->dev_prop_name_of_station.name) > block_len - 3);
        SPN_DCP_BLOCK_TOUCH(&block->dev_prop_name_of_station);
        strncpy(block->dev_prop_name_of_station.name, r_payload + 2, block_len - 2);
        block->dev_prop_name_of_station.name[block_len - 2] = '\0';
        block->dev_prop_name_of_station.name_len = block_len - 2;
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: NameOfStation=%s\n", block->dev_prop_name_of_station.name));
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ID):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.DeviceID\n"));
        LWIP_ASSERT("Invalid block length", block_len == 6);
        SPN_DCP_BLOCK_TOUCH(&block->dev_prop_device_id);
        block->dev_prop_device_id.vendor_id = lwip_ntohs(GET_VALUE(r_payload, uint16_t, 2));
        block->dev_prop_device_id.device_id = lwip_ntohs(GET_VALUE(r_payload, uint16_t, 4));
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: DeviceID=0x%04x:0x%04x\n", block->dev_prop_device_id.vendor_id, block->dev_prop_device_id.device_id));
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ROLE):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.DeviceRole\n"));
        LWIP_ASSERT("Invalid block length", block_len == 4); /* RoleBLock has 1 byte padding */
        SPN_DCP_BLOCK_TOUCH(&block->dev_prop_role);
        block->dev_prop_role.role = GET_VALUE(r_payload, uint8_t, 2);
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: DeviceRole=%d\n", block->dev_prop_role.role));
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_OPTIONS):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.DeviceOptions\n"));
        LWIP_ASSERT("Invalid block length", block_len >= 2);
        SPN_DCP_BLOCK_TOUCH(&block->dev_prop_device_options);
        {
            uint16_t* block_info = (uint16_t*)((uintptr_t)r_payload + 2);
            int left_index = (block_len - 2) / 2;
            int i;

            LWIP_ASSERT("DeviceOptions too long", left_index <= (int)(sizeof(block->dev_prop_device_options.options) / sizeof(block->dev_prop_device_options.options[0])));
            for (i = 0; i < left_index; i++) {
                block->dev_prop_device_options.options[i] = lwip_ntohs(block_info[i]);
                LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: DeviceOptions[%d]=0x%04x\n", i, lwip_ntohs(block_info[i])));
            }
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: DeviceOptions num=%d\n", left_index));
            block->dev_prop_device_options.option_num = left_index;
        }
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_ALIAS_NAME):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.AliasName\n"));
        LWIP_ASSERT("AliasName too long", (int)sizeof(block->dev_prop_alias_name.alias_name) > block_len - 3);
        SPN_DCP_BLOCK_TOUCH(&block->dev_prop_alias_name);
        strncpy(block->dev_prop_alias_name.alias_name, r_payload + 2, block_len - 2);
        block->dev_prop_alias_name.alias_name[block_len - 2] = '\0';
        block->dev_prop_alias_name.alias_name_len = block_len - 2;
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: AliasName=%s\n", block->dev_prop_alias_name.alias_name));
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_INSTANCE):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.DeviceInstance\n"));
        LWIP_ASSERT("Invalid block length", block_len == 4);
        SPN_DCP_BLOCK_TOUCH(&block->dev_prop_dev_instance);
        block->dev_prop_dev_instance.instance = lwip_ntohs(GET_VALUE(r_payload, uint16_t, 2));
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: DeviceInstance=0x%04x\n", block->dev_prop_dev_instance.instance));
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_OEM_DEVICE_ID):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.OEMDeviceID\n"));
        LWIP_ASSERT("Invalid block length", block_len == 6);
        SPN_DCP_BLOCK_TOUCH(&block->dev_prop_dev_oem_id);
        block->dev_prop_dev_oem_id.vendor_id = lwip_ntohs(GET_VALUE(r_payload, uint16_t, 2));
        block->dev_prop_dev_oem_id.device_id = lwip_ntohs(GET_VALUE(r_payload, uint16_t, 4));
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: OEMDeviceID=0x%04x:0x%04x\n", block->dev_prop_dev_oem_id.vendor_id, block->dev_prop_dev_oem_id.device_id));
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_STANDARD_GATEWAY):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.StandardGateway\n"));
        LWIP_ASSERT("Invalid block length", block_len == 4);
        SPN_DCP_BLOCK_TOUCH(&block->ip_param);
        block->dev_prop_dev_std_gateway.standard_gateway = lwip_ntohs(GET_VALUE(r_payload, uint16_t, 2));
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: StandardGateway=0x%02x\n", block->dev_prop_dev_std_gateway.standard_gateway));
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_RSI_PROPERTIES):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Device.Properties.RSIProperties\n"));
        LWIP_ASSERT("Invalid block length", block_len == 4);
        SPN_DCP_BLOCK_TOUCH(&block->dev_prop_rsi_prop);
        block->dev_prop_rsi_prop.rsi_prop_value = lwip_ntohs(GET_VALUE(r_payload, uint16_t, 2));
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: RSIProperties=0x%02x\n", block->dev_prop_rsi_prop.rsi_prop_value));
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_DHCP, SPN_DCP_SUB_OPT_DHCP_DHCP):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing DHCP.DHCP\n"));
        LWIP_ASSERT("Invalid block length", block_len == 5);
        SPN_DCP_BLOCK_TOUCH(&block->dhcp);
        block->dhcp.option = GET_VALUE(r_payload, uint8_t, 2);
        block->dhcp.dhcp_param_length = GET_VALUE(r_payload, uint8_t, 3);
        block->dhcp.dhcp_param_data[0] = GET_VALUE(r_payload, uint8_t, 4);
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: DHCP.Option=%d, DHCP.ParamLength=%d, DHCP.ParamData=%d\n", block->dhcp.option, block->dhcp.dhcp_param_length, block->dhcp.dhcp_param_data[0]));
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_CONTROL, SPN_DCP_SUB_OPT_CONTROL_START):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Control.Start\n"));
        /* Empty block */
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_CONTROL, SPN_DCP_SUB_OPT_CONTROL_STOP):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Control.Stop\n"));
        /* Empty block */
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_CONTROL, SPN_DCP_SUB_OPT_CONTROL_SIGNAL):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Control.Signal\n"));
        /* Empty block */
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_CONTROL, SPN_DCP_SUB_OPT_CONTROL_RESPONSE):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Control.Response\n"));
        /* Empty block */
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_CONTROL, SPN_DCP_SUB_OPT_CONTROL_FACTORY_RESET):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Control.FactoryReset\n"));
        /* Empty block */
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_CONTROL, SPN_DCP_SUB_OPT_CONTROL_RESET_TO_FACTORY):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing Control.ResetToFactory\n"));
        /* Empty block */
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_INITIATIVE, SPN_DCP_SUB_OPT_DEVICE_INITIATIVE_DEVICE_INITIATIVE):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing DeviceInitiative.DeviceInitiative\n"));
        LWIP_ASSERT("Invalid block length", block_len == 4);
        SPN_DCP_BLOCK_TOUCH(&block->dev_initiative);
        block->dev_initiative.value = lwip_ntohs(GET_VALUE(r_payload, uint16_t, 2));
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: DeviceInitiative=0x%04x\n", block->dev_initiative.value));
        break;
#if 0 /* For TSN feature, not supported */
    case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_DOMAIN):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing NMEDomain.NMEDomain\n"));
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_PRIO):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing NMEDomain.NMEPrio\n"));
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_PARAMETER_UUID):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing NMEDomain.NMEParameterUUID\n"));
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_NAME):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing NMEDomain.NMEName\n"));
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_CIM_INTERFACE):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing NMEDomain.CIMInterface\n"));
        break;
    case BLOCK_TYPE(SPN_DCP_OPTION_ALL_SELECTOR, SPN_DCP_SUB_OPT_ALL_SELECTOR_ALL_SELECTOR):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing AllSelector.AllSelector\n"));
        break;
#endif
    default:
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: unknown block_type=0x%04x\n", block_type));
    }

    offset = spn_dcp_block_walk(payload, offset);
    return spn_dcp_block_parse(payload, len, offset, deep + 1, block);
}

int spn_dcp_input(void* frame, size_t len, uint16_t frame_id, struct eth_hdr* hw_hdr, iface_t* iface)
{
    struct spn_dcp_header* dcp_hdr = (struct spn_dcp_header*)frame;
    struct spn_dcp_general_block* dcp_blocks = (struct spn_dcp_general_block*)((uint8_t*)dcp_hdr + sizeof(struct spn_dcp_header));
    uint16_t dcp_data_len = PP_HTONS(dcp_hdr->dcp_data_length);
    uint32_t dcp_xid = PP_HTONL(dcp_hdr->xid);
    uint8_t dcp_service_id = dcp_hdr->service_id;
    uint8_t dcp_service_type = dcp_hdr->service_type;
    struct spn_dcp_block blocks;
    int res;

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
    case FRAME_ID_DCP_IDENT_REQ: {
        struct spn_dcp_ident_req req;
        /* Available type:
         * IdentifyAll-Req(NameOfStationBlock^AliasNameBlock^IdentifyReqBlock)
         * IdentifyFilter-Req(AllSelectorBlock) */
        if (dcp_service_id != SPN_DCP_SERVICE_ID_IDENTIFY) {
            goto err_invalid_service_id;
        }
        res = spn_dcp_ident_req_parse(dcp_blocks, dcp_data_len, &req);

        if (res != SPN_OK) {
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: spn_dcp_ident_req_parse failed, res=%d\n", res));
            break;
        }
        break;
    }
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
    spn_dcp_block_parse(dcp_blocks, dcp_data_len, 0, 0, &blocks);

    /* TODO: Handle blocks */

    return SPN_OK;

err_invalid_service_id:
    LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_HALT, ("DCP: invalid service_id=%d\n", dcp_service_id));
    /* TODO: send error response */
    return SPN_OK;
}

int spn_dcp_ident_req_parse(void* payload, uint16_t len, struct spn_dcp_ident_req* reqs)
{
    const struct spn_dcp_general_block* block_ptr;
    uint16_t offset;
    uint16_t block_len;
    uint16_t block_type;
    const char* r_payload;
    unsigned int idx;

    /* temps for filter */
    uint32_t ip_addr, ip_mask, ip_gw, ip_dns[4];
    uint16_t name_len, vendor_id, device_id, tmp_u16;
    const char* name;

    idx = 0;
    r_payload = (const char*)payload;

    for (offset = 0; offset < len;) {
        LWIP_ASSERT("DCP: At least 16bit align", (offset & 2) == 0);
        r_payload = (const char*)((uintptr_t)payload + offset);
        block_ptr = (const struct spn_dcp_general_block*)r_payload;
        block_len = lwip_htons(block_ptr->dcp_block_length);
        block_type = BLOCK_TYPE(block_ptr->option, block_ptr->sub_option);

        if (block_len + sizeof(*block_ptr) + offset > len) {
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_HALT, ("DCP: invalid block_len=%d, len=%d, offset=%d\n", block_len, len, offset));
            return -SPN_EBADMSG;
        }

        switch (block_type) {
        case BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_PARAMETER):
            LWIP_ASSERT("Invalid block length", block_len == 14);
            ip_addr = lwip_htonl(GET_VALUE(r_payload, uint32_t, 0));
            ip_mask = lwip_htonl(GET_VALUE(r_payload, uint32_t, 4));
            ip_gw = lwip_htonl(GET_VALUE(r_payload, uint32_t, 8));
            if (spn_dcp_filter_ip(ip_addr, ip_mask, ip_gw)) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_FULL_IP_SUITE):
            LWIP_ASSERT("Invalid block length", block_len == 30);
            ip_addr = lwip_htonl(GET_VALUE(r_payload, uint32_t, 0));
            ip_mask = lwip_htonl(GET_VALUE(r_payload, uint32_t, 4));
            ip_gw = lwip_htonl(GET_VALUE(r_payload, uint32_t, 8));
            ip_dns[0] = lwip_htons(GET_VALUE(r_payload, uint32_t, 12));
            ip_dns[1] = lwip_htons(GET_VALUE(r_payload, uint32_t, 16));
            ip_dns[2] = lwip_htons(GET_VALUE(r_payload, uint32_t, 20));
            ip_dns[3] = lwip_htons(GET_VALUE(r_payload, uint32_t, 24));
            if (spn_dcp_filter_ip(ip_addr, ip_mask, ip_gw)) {
                goto filter_miss_match;
            }
            if (spn_dcp_filter_dns(ip_dns[0], ip_dns[1], ip_dns[2], ip_dns[3])) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_VENDOR):
            LWIP_ASSERT("Invalid block length", block_len >= 1);
            name = r_payload;
            name_len = block_len;
            if (spn_dcp_filter_vendor_name(name, name_len)) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_NAME_OF_STATION):
            LWIP_ASSERT("Invalid block length", block_len >= 1);
            name = r_payload;
            name_len = block_len;
            if (spn_dcp_filter_station_of_name(name, name_len)) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ID):
            LWIP_ASSERT("Invalid block length", block_len == 4);
            vendor_id = lwip_htons(GET_VALUE(r_payload, uint16_t, 0));
            device_id = lwip_htons(GET_VALUE(r_payload, uint16_t, 2));
            if (spn_dcp_filter_vendor_id(vendor_id, device_id)) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ROLE):
            LWIP_ASSERT("Invalid block length", block_len == 2);
            if (spn_dcp_filter_role(GET_VALUE(r_payload, uint8_t, 0))) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_OPTIONS):
            LWIP_ASSERT("Invalid block length", block_len >= 2);
            if (spn_dcp_filter_options((const uint16_t*)r_payload, block_len / 2)) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_ALIAS_NAME):
            LWIP_ASSERT("Invalid block length", block_len >= 1);
            if (spn_dcp_filter_alias(r_payload, block_len)) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_INSTANCE):
            LWIP_ASSERT("Invalid block length", block_len == 2);
            tmp_u16 = lwip_htons(GET_VALUE(r_payload, uint16_t, 0));
            if (spn_dcp_filter_instance(tmp_u16)) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_OEM_DEVICE_ID):
            LWIP_ASSERT("Invalid block length", block_len == 4);
            vendor_id = lwip_htons(GET_VALUE(r_payload, uint16_t, 0));
            device_id = lwip_htons(GET_VALUE(r_payload, uint16_t, 2));
            if (spn_dcp_filter_oem_id(vendor_id, device_id)) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_STANDARD_GATEWAY):
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_RSI_PROPERTIES):
        case BLOCK_TYPE(SPN_DCP_OPTION_DHCP, SPN_DCP_SUB_OPT_DHCP_DHCP):
            return -SPN_ENOSYS;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_INITIATIVE, SPN_DCP_SUB_OPT_DEVICE_INITIATIVE_DEVICE_INITIATIVE):
            LWIP_ASSERT("Invalid block length", block_len == 2);
            tmp_u16 = lwip_htons(GET_VALUE(r_payload, uint16_t, 0));
            if (spn_dcp_filter_device_initiative(tmp_u16)) {
                goto filter_miss_match;
            }
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_DOMAIN):
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_PRIO):
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_NAME):
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_CIM_INTERFACE):
            return -SPN_ENOSYS;
        case BLOCK_TYPE(SPN_DCP_OPTION_ALL_SELECTOR, SPN_DCP_SUB_OPT_ALL_SELECTOR_ALL_SELECTOR):
            break;
        default:
            goto filter_miss_match;
        }

        /* Push option into reqs */
        if (idx >= sizeof(reqs->option_sub_option) / sizeof(reqs->option_sub_option[0])) {
            LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP.ident_req_parse: too many blocks\n"));
            return -SPN_EMSGSIZE;
        }
        reqs->option_sub_option[idx++] = block_type;

        /* Goto next req block */
        offset = spn_dcp_block_walk(payload, offset);
    }

    return SPN_OK;
filter_miss_match:
    LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP.ident_req_parse: filter miss match, last block_type=0x%04x\n", block_type));
    return -SPN_ENXIO;
}

static const uint16_t spn_dcp_mandatory_reqs[] = {
    BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_PARAMETER),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_VENDOR),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_NAME_OF_STATION),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ID),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ROLE),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_OPTIONS),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_INSTANCE),
};

int spn_dcp_ident_resp_assemble(struct eth_hdr* hw_hdr, struct spn_dcp_ident_req* reqs, iface_t* iface)
{
    struct pbuf* p;
    char* r_payload;
    int res = SPN_OK;
    int len;
    uint16_t offset = 0;
    const int16_t header_size = sizeof(struct spn_dcp_header) + sizeof(struct pn_pdu) + SIZEOF_ETH_HDR;
    const uint16_t hdr_size = sizeof(struct spn_dcp_general_block) + 2;
    uint16_t block_info = spn_dcp_pack_block_info();
    struct spn_dcp_header* dcp_hdr;
    struct pn_pdu* pn_hdr;

    /* merge two list into one in asc order */
    uint16_t merged[32];
    unsigned i = 0, j = 0, k = 0;
    while (i < sizeof(spn_dcp_mandatory_reqs) / sizeof(spn_dcp_mandatory_reqs[0]) && j < sizeof(reqs->option_sub_option) / sizeof(reqs->option_sub_option[0])) {
        if (spn_dcp_mandatory_reqs[i] < reqs->option_sub_option[j]) {
            merged[k++] = spn_dcp_mandatory_reqs[i++];
        } else {
            merged[k++] = reqs->option_sub_option[j++];
        }
    }

    p = pbuf_alloc(PBUF_RAW, SPN_DCP_DATA_MAX_LENGTH + SIZEOF_ETH_HDR + sizeof(struct pn_pdu), PBUF_RAM);
    if (!p) {
        res = -SPN_ENOMEM;
        goto out;
    }
    pbuf_header(p, -header_size);
    r_payload = (char*)p->payload;

    /* Fill blocks */
    for (i = 0; i < k; i++) {
        uint16_t block_type = merged[i];

        switch (block_type) {
        case BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_PARAMETER):
            spn_dcp_pack_block(r_payload + offset, block_type, 12, block_info);
            spn_dcp_pack_ip(r_payload + offset + hdr_size);
            offset += hdr_size + 12;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_FULL_IP_SUITE):
            spn_dcp_pack_block(r_payload + offset, block_type, 28, block_info);
            spn_dcp_pack_ip(r_payload + offset + hdr_size);
            spn_dcp_pack_dns(r_payload + offset + hdr_size + 12);
            offset += hdr_size + 12;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_VENDOR):
            len = spn_dcp_pack_vendor_name(r_payload + offset + hdr_size);
            spn_dcp_pack_block(r_payload + offset, block_type, len, block_info);
            offset += hdr_size + len;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_NAME_OF_STATION):
            len = spn_dcp_pack_station_of_name(r_payload + offset + hdr_size);
            spn_dcp_pack_block(r_payload + offset, block_type, len, block_info);
            offset += hdr_size + len;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ID):
            spn_dcp_pack_block(r_payload + offset, block_type, 4, block_info);
            spn_dcp_pack_device_id(r_payload + offset + hdr_size);
            offset += hdr_size + 4;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ROLE):
            spn_dcp_pack_block(r_payload + offset, block_type, 2, block_info);
            spn_dcp_pack_role(r_payload + offset + hdr_size);
            offset += hdr_size + 2;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_OPTIONS):
            len = spn_dcp_pack_options(r_payload + offset + hdr_size);
            spn_dcp_pack_block(r_payload + offset, block_type, len, block_info);
            offset += hdr_size + len;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_ALIAS_NAME):
            len = spn_dcp_pack_alias(r_payload + offset + hdr_size);
            spn_dcp_pack_block(r_payload + offset, block_type, len, block_info);
            offset += hdr_size + len;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_INSTANCE):
            spn_dcp_pack_block(r_payload + offset, block_type, 2, block_info);
            spn_dcp_pack_instance(r_payload + offset + hdr_size);
            offset += hdr_size + 2;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_OEM_DEVICE_ID):
            spn_dcp_pack_block(r_payload + offset, block_type, 4, block_info);
            spn_dcp_pack_oem_id(r_payload + offset + hdr_size);
            offset += hdr_size + 4;
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_STANDARD_GATEWAY):
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_RSI_PROPERTIES):
        case BLOCK_TYPE(SPN_DCP_OPTION_DHCP, SPN_DCP_SUB_OPT_DHCP_DHCP):
            res = -SPN_ENOSYS;
            goto free_out;
        case BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_INITIATIVE, SPN_DCP_SUB_OPT_DEVICE_INITIATIVE_DEVICE_INITIATIVE):
            break;
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_DOMAIN):
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_PRIO):
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_NME_NAME):
        case BLOCK_TYPE(SPN_DCP_OPTION_NME_DOMAIN, SPN_DCP_SUB_OPT_NME_DOMAIN_CIM_INTERFACE):
            res = -SPN_ENOSYS;
            goto free_out;
        case BLOCK_TYPE(SPN_DCP_OPTION_ALL_SELECTOR, SPN_DCP_SUB_OPT_ALL_SELECTOR_ALL_SELECTOR):
            break;
        default:
            res = -SPN_ENOSYS;
            goto free_out;
        }

        /* Align to 16-bit */
        offset = (offset + 1) & ~1;
        if (offset > SPN_DCP_DATA_MAX_LENGTH) {
            res = -SPN_EMSGSIZE;
            goto free_out;
        }
    }

    pbuf_header(p, sizeof(*dcp_hdr));
    dcp_hdr = (struct spn_dcp_header*)p->payload;
    dcp_hdr->service_id = SPN_DCP_SERVICE_ID_IDENTIFY;
    dcp_hdr->service_type = SPN_DCP_SERVICE_TYPE_RESPONSE;
    dcp_hdr->dcp_data_length = lwip_htons(offset);
    dcp_hdr->xid = lwip_htonl(0x12345678); /* TODO: Resp the same xid as ident.req */

    pbuf_header(p, sizeof(*pn_hdr));
    pn_hdr = (struct pn_pdu*)p->payload;
    pn_hdr->frame_id = PP_HTONS(FRAME_ID_DCP_IDENT_REQ);

    ethernet_output(iface, p, (const struct eth_addr*)iface->hwaddr, &hw_hdr->src, ETHTYPE_PROFINET);
free_out:
    pbuf_free(p);
out:
    return res;
}
