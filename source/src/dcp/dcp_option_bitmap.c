#include <spn/dcp.h>
#include <spn/errno.h>

#define BLOCK_TYPE(option, sub_option) ((option) << 8 | (sub_option))

int dcp_option_bit_idx(uint8_t option, uint8_t sub_option) {
  switch (BLOCK_TYPE(option, sub_option)) {
    case BLOCK_TYPE(DCP_OPT_IP, DCP_SUB_OPT_IP_MAC):
      return DCP_BIT_IDX_IP_MAC_ADDRESS;
    case BLOCK_TYPE(DCP_OPT_IP, DCP_SUB_OPT_IP_PARAM):
      return DCP_BIT_IDX_IP_PARAMETER;
    case BLOCK_TYPE(DCP_OPT_IP, DCP_SUB_OPT_IP_FULL_SUITE):
      return DCP_BIT_IDX_IP_FULL_IP_SUITE;
    case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR):
      return DCP_BIT_IDX_DEV_PROP_NAME_OF_VENDOR;
    case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION):
      return DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION;
    case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ID):
      return DCP_BIT_IDX_DEV_PROP_DEVICE_ID;
    case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ROLE):
      return DCP_BIT_IDX_DEV_PROP_DEVICE_ROLE;
    case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_OPTIONS):
      return DCP_BIT_IDX_DEV_PROP_DEVICE_OPTIONS;
    case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_ALIAS):
      return DCP_BIT_IDX_DEV_PROP_NAME_OF_ALIAS;
    case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_INSTANCE):
      return DCP_BIT_IDX_DEV_PROP_DEVICE_INSTANCE;
    case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_OEM_DEVICE_ID):
      return DCP_BIT_IDX_DEV_PROP_OEM_DEVICE_ID;
    case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_STD_GATEWAY):
      return DCP_BIT_IDX_DEV_PROP_STANDARD_GATEWAY;
    case BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_RSI):
      return DCP_BIT_IDX_DEV_PROP_RSI_PROPERTIES;
    case BLOCK_TYPE(DCP_OPT_DHCP, DCP_SUB_OPT_DHCP_CLIENT_IDENT):
      return DCP_BIT_IDX_DHCP_CLIENT_IDENT;
    case BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_START):
      return DCP_BIT_IDX_CTRL_START;
    case BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_STOP):
      return DCP_BIT_IDX_CTRL_STOP;
    case BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_SIGNAL):
      return DCP_BIT_IDX_CTRL_SIGNAL;
    case BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_RESPONSE):
      return DCP_BIT_IDX_CTRL_RESPONSE;
    case BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_FACTORY_RESET):
      return DCP_BIT_IDX_CTRL_FACTORY_RESET;
    case BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_RESET_TO_FACTORY):
      return DCP_BIT_IDX_CTRL_RESET_TO_FACTORY;
    case BLOCK_TYPE(DCP_OPT_DEV_INITIATIVE, DCP_SUB_OPT_DEV_INITIATIVE):
      return DCP_BIT_IDX_DEVICE_INITIATIVE;
    case BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_DOMAIN):
      return DCP_BIT_IDX_NME_DOMAIN_NME_DOMAIN;
    case BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_PRIORITY):
      return DCP_BIT_IDX_NME_DOMAIN_NME_PRIO;
    case BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_PARAM_UUID):
      return DCP_BIT_IDX_NME_DOMAIN_NME_PARAMETER_UUID;
    case BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_NAME):
      return DCP_BIT_IDX_NME_DOMAIN_NME_NAME;
    case BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_CIM_INTERFACE):
      return DCP_BIT_IDX_NME_DOMAIN_CIM_INTERFACE;
    case BLOCK_TYPE(DCP_OPT_ALL_SELECTOR, DCP_SUB_OPT_ALL_SELECTOR):
      return DCP_BIT_IDX_ALL_SELECTOR;
    default:
      return -1;
  }
}

uint16_t dcp_option_from_bit_idx(uint32_t offset) {
  switch (offset) {
    case DCP_BIT_IDX_IP_MAC_ADDRESS:
      return BLOCK_TYPE(DCP_OPT_IP, DCP_SUB_OPT_IP_MAC);
    case DCP_BIT_IDX_IP_PARAMETER:
      return BLOCK_TYPE(DCP_OPT_IP, DCP_SUB_OPT_IP_PARAM);
    case DCP_BIT_IDX_IP_FULL_IP_SUITE:
      return BLOCK_TYPE(DCP_OPT_IP, DCP_SUB_OPT_IP_FULL_SUITE);
    case DCP_BIT_IDX_DEV_PROP_NAME_OF_VENDOR:
      return BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR);
    case DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION:
      return BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION);
    case DCP_BIT_IDX_DEV_PROP_DEVICE_ID:
      return BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ID);
    case DCP_BIT_IDX_DEV_PROP_DEVICE_ROLE:
      return BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ROLE);
    case DCP_BIT_IDX_DEV_PROP_DEVICE_OPTIONS:
      return BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_OPTIONS);
    case DCP_BIT_IDX_DEV_PROP_NAME_OF_ALIAS:
      return BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_ALIAS);
    case DCP_BIT_IDX_DEV_PROP_DEVICE_INSTANCE:
      return BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_INSTANCE);
    case DCP_BIT_IDX_DEV_PROP_OEM_DEVICE_ID:
      return BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_OEM_DEVICE_ID);
    case DCP_BIT_IDX_DEV_PROP_STANDARD_GATEWAY:
      return BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_STD_GATEWAY);
    case DCP_BIT_IDX_DEV_PROP_RSI_PROPERTIES:
      return BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_RSI);
    case DCP_BIT_IDX_DHCP_CLIENT_IDENT:
      return BLOCK_TYPE(DCP_OPT_DHCP, DCP_SUB_OPT_DHCP_CLIENT_IDENT);
    case DCP_BIT_IDX_CTRL_START:
      return BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_START);
    case DCP_BIT_IDX_CTRL_STOP:
      return BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_STOP);
    case DCP_BIT_IDX_CTRL_SIGNAL:
      return BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_SIGNAL);
    case DCP_BIT_IDX_CTRL_RESPONSE:
      return BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_RESPONSE);
    case DCP_BIT_IDX_CTRL_FACTORY_RESET:
      return BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_FACTORY_RESET);
    case DCP_BIT_IDX_CTRL_RESET_TO_FACTORY:
      return BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_RESET_TO_FACTORY);
    case DCP_BIT_IDX_DEVICE_INITIATIVE:
      return BLOCK_TYPE(DCP_OPT_DEV_INITIATIVE, DCP_SUB_OPT_DEV_INITIATIVE);
    case DCP_BIT_IDX_NME_DOMAIN_NME_DOMAIN:
      return BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_DOMAIN);
    case DCP_BIT_IDX_NME_DOMAIN_NME_PRIO:
      return BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_PRIORITY);
    case DCP_BIT_IDX_NME_DOMAIN_NME_PARAMETER_UUID:
      return BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_PARAM_UUID);
    case DCP_BIT_IDX_NME_DOMAIN_NME_NAME:
      return BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_NAME);
    case DCP_BIT_IDX_NME_DOMAIN_CIM_INTERFACE:
      return BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_CIM_INTERFACE);
    case DCP_BIT_IDX_ALL_SELECTOR:
      return BLOCK_TYPE(DCP_OPT_ALL_SELECTOR, DCP_SUB_OPT_ALL_SELECTOR);
    default:
      return 0;
  }
}
