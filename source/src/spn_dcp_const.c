#include <spn/dcp.h>

#define BLOCK_TYPE(option, sub_option) ((option << 8) | sub_option)
#define GET_VALUE(ptr, type, offset) (*(type*)((uintptr_t)ptr + offset))

const uint16_t _dcp_filter_blocks[] = {
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
    BLOCK_TYPE(0, 0),
};
const uint16_t *dcp_filter_blocks = _dcp_filter_blocks;

const uint16_t _spn_dcp_mandatory_reqs[] = {
    BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_PARAMETER),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_VENDOR),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_NAME_OF_STATION),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ID),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ROLE),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_OPTIONS),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_INSTANCE),
    BLOCK_TYPE(0, 0),
};
const uint16_t *spn_dcp_mandatory_reqs = &_spn_dcp_mandatory_reqs[0];

const uint16_t _spn_dcp_supported_options[] = {
    BLOCK_TYPE(SPN_DCP_OPTION_IP, SPN_DCP_SUB_OPT_IP_PARAMETER),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_VENDOR),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_NAME_OF_STATION),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ID),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_ROLE),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_OPTIONS),
    BLOCK_TYPE(SPN_DCP_OPTION_DEVICE_PROPERTIES, SPN_DCP_SUB_OPT_DEVICE_PROPERTIES_DEVICE_INSTANCE),
    BLOCK_TYPE(0, 0),
};
const uint16_t *spn_dcp_supported_options = _spn_dcp_supported_options;
