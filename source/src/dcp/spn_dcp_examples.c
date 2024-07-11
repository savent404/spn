#include <spn/dcp.h>
#include <spn/errno.h>
#include <stdlib.h>
#include <string.h>

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
#endif
    case BLOCK_TYPE(SPN_DCP_OPTION_ALL_SELECTOR, SPN_DCP_SUB_OPT_ALL_SELECTOR_ALL_SELECTOR):
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: Parsing AllSelector.AllSelector\n"));
        break;
    default:
        LWIP_DEBUGF(SPN_DCP_DEBUG | LWIP_DBG_TRACE, ("DCP: unknown block_type=0x%04x\n", block_type));
    }

    offset = spn_dcp_block_walk(payload, offset);
    return spn_dcp_block_parse(payload, len, offset, deep + 1, block);
}
