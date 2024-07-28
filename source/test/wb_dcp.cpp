#include <gtest/gtest.h>
#include <spn/dcp.h>

#define BLOCK_TYPE(option, sub_option) ((option) << 8 | (sub_option))

TEST(dcp_option_bitmap, maxmium_bit_num) {
  ASSERT_LT(DCP_BIT_IDX_NUM, 32);
}

/**
 * This is all about coverage testing, so we don't need to test the actual functionality of these functions.
 */
TEST(dcp_option_bitmap, dcp_option_bitmap_ip_mac_address) {
  // Test case for BLOCK_TYPE(DCP_OPT_IP, DCP_SUB_OPT_IP_MAC)
  uint32_t result1 = dcp_option_bitmap(DCP_OPT_IP, DCP_SUB_OPT_IP_MAC);
  EXPECT_EQ(result1, DCP_BIT_IDX_IP_MAC_ADDRESS);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_ip_parameter) {
  // Test case for BLOCK_TYPE(DCP_OPT_IP, DCP_SUB_OPT_IP_PARAM)
  uint32_t result2 = dcp_option_bitmap(DCP_OPT_IP, DCP_SUB_OPT_IP_PARAM);
  EXPECT_EQ(result2, DCP_BIT_IDX_IP_PARAMETER);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_ip_full_ip_suite) {
  // Test case for BLOCK_TYPE(DCP_OPT_IP, DCP_SUB_OPT_IP_FULL_SUITE)
  uint32_t result3 = dcp_option_bitmap(DCP_OPT_IP, DCP_SUB_OPT_IP_FULL_SUITE);
  EXPECT_EQ(result3, DCP_BIT_IDX_IP_FULL_IP_SUITE);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_device_properties_name_of_vendor) {
  // Test case for BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR)
  uint32_t result4 = dcp_option_bitmap(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR);
  EXPECT_EQ(result4, DCP_BIT_IDX_DEV_PROP_NAME_OF_VENDOR);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_device_properties_name_of_station) {
  // Test case for BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION)
  uint32_t result5 = dcp_option_bitmap(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION);
  EXPECT_EQ(result5, DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_device_properties_device_id) {
  // Test case for BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ID)
  uint32_t result6 = dcp_option_bitmap(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ID);
  EXPECT_EQ(result6, DCP_BIT_IDX_DEV_PROP_DEVICE_ID);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_device_properties_device_role) {
  // Test case for BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ROLE)
  uint32_t result7 = dcp_option_bitmap(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ROLE);
  EXPECT_EQ(result7, DCP_BIT_IDX_DEV_PROP_DEVICE_ROLE);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_device_properties_device_options) {
  // Test case for BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_OPTIONS)
  uint32_t result8 = dcp_option_bitmap(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_OPTIONS);
  EXPECT_EQ(result8, DCP_BIT_IDX_DEV_PROP_DEVICE_OPTIONS);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_device_properties_name_of_alias) {
  // Test case for BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_ALIAS)
  uint32_t result9 = dcp_option_bitmap(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_ALIAS);
  EXPECT_EQ(result9, DCP_BIT_IDX_DEV_PROP_NAME_OF_ALIAS);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_device_properties_device_instance) {
  // Test case for BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_INSTANCE)
  uint32_t result10 = dcp_option_bitmap(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_INSTANCE);
  EXPECT_EQ(result10, DCP_BIT_IDX_DEV_PROP_DEVICE_INSTANCE);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_device_properties_oem_device_id) {
  // Test case for BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_OEM_DEVICE_ID)
  uint32_t result11 = dcp_option_bitmap(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_OEM_DEVICE_ID);
  EXPECT_EQ(result11, DCP_BIT_IDX_DEV_PROP_OEM_DEVICE_ID);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_device_properties_standard_gateway) {
  // Test case for BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_STD_GATEWAY)
  uint32_t result12 = dcp_option_bitmap(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_STD_GATEWAY);
  EXPECT_EQ(result12, DCP_BIT_IDX_DEV_PROP_STANDARD_GATEWAY);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_device_properties_rsi_properties) {
  // Test case for BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_RSI)
  uint32_t result13 = dcp_option_bitmap(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_RSI);
  EXPECT_EQ(result13, DCP_BIT_IDX_DEV_PROP_RSI_PROPERTIES);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_dhcp_client_ident) {
  // Test case for BLOCK_TYPE(DCP_OPT_DHCP, DCP_SUB_OPT_DHCP_CLIENT_IDENT)
  uint32_t result14 = dcp_option_bitmap(DCP_OPT_DHCP, DCP_SUB_OPT_DHCP_CLIENT_IDENT);
  EXPECT_EQ(result14, DCP_BIT_IDX_DHCP_CLIENT_IDENT);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_control_start) {
  // Test case for BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_START)
  uint32_t result15 = dcp_option_bitmap(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_START);
  EXPECT_EQ(result15, DCP_BIT_IDX_CTRL_START);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_control_stop) {
  // Test case for BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_STOP)
  uint32_t result16 = dcp_option_bitmap(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_STOP);
  EXPECT_EQ(result16, DCP_BIT_IDX_CTRL_STOP);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_control_signal) {
  // Test case for BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_SIGNAL)
  uint32_t result17 = dcp_option_bitmap(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_SIGNAL);
  EXPECT_EQ(result17, DCP_BIT_IDX_CTRL_SIGNAL);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_control_response) {
  // Test case for BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_RESPONSE)
  uint32_t result18 = dcp_option_bitmap(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_RESPONSE);
  EXPECT_EQ(result18, DCP_BIT_IDX_CTRL_RESPONSE);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_control_factory_reset) {
  // Test case for BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_FACTORY_RESET)
  uint32_t result19 = dcp_option_bitmap(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_FACTORY_RESET);
  EXPECT_EQ(result19, DCP_BIT_IDX_CTRL_FACTORY_RESET);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_control_reset_to_factory) {
  // Test case for BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_RESET_TO_FACTORY)
  uint32_t result20 = dcp_option_bitmap(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_RESET_TO_FACTORY);
  EXPECT_EQ(result20, DCP_BIT_IDX_CTRL_RESET_TO_FACTORY);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_device_initiative_device_initiative) {
  // Test case for BLOCK_TYPE(DCP_OPT_DEV_INITIATIVE, DCP_SUB_OPT_DEV_INITIATIVE)
  uint32_t result21 = dcp_option_bitmap(DCP_OPT_DEV_INITIATIVE, DCP_SUB_OPT_DEV_INITIATIVE);
  EXPECT_EQ(result21, DCP_BIT_IDX_DEVICE_INITIATIVE);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_nme_domain_nme_domain) {
  // Test case for BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_DOMAIN)
  uint32_t result22 = dcp_option_bitmap(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_DOMAIN);
  EXPECT_EQ(result22, DCP_BIT_IDX_NME_DOMAIN_NME_DOMAIN);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_nme_domain_nme_prio) {
  // Test case for BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_PRIORITY)
  uint32_t result23 = dcp_option_bitmap(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_PRIORITY);
  EXPECT_EQ(result23, DCP_BIT_IDX_NME_DOMAIN_NME_PRIO);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_nme_domain_nme_parameter_uuid) {
  // Test case for BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_PARAM_UUID)
  uint32_t result24 = dcp_option_bitmap(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_PARAM_UUID);
  EXPECT_EQ(result24, DCP_BIT_IDX_NME_DOMAIN_NME_PARAMETER_UUID);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_nme_domain_nme_name) {
  // Test case for BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_NAME)
  uint32_t result25 = dcp_option_bitmap(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_NAME);
  EXPECT_EQ(result25, DCP_BIT_IDX_NME_DOMAIN_NME_NAME);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_nme_domain_cim_interface) {
  // Test case for BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_CIM_INTERFACE)
  uint32_t result26 = dcp_option_bitmap(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_CIM_INTERFACE);
  EXPECT_EQ(result26, DCP_BIT_IDX_NME_DOMAIN_CIM_INTERFACE);
}

TEST(dcp_option_bitmap, dcp_option_bitmap_all_selector) {
  // Test case for BLOCK_TYPE(DCP_OPT_ALL_SELECTOR, DCP_SUB_OPT_ALL_SELECTOR)
  uint32_t result27 = dcp_option_bitmap(DCP_OPT_ALL_SELECTOR, DCP_SUB_OPT_ALL_SELECTOR);
  EXPECT_EQ(result27, DCP_BIT_IDX_ALL_SELECTOR);
}

TEST(dcp_option_bitmap, unknow) {
  // Test case for unknown option
  uint32_t result28 = dcp_option_bitmap(0x00, 0x00);
  EXPECT_EQ(result28, -1);
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_IP_MAC_ADDRESS) {
  uint16_t result1 = dcp_option_bit_offset(DCP_BIT_IDX_IP_MAC_ADDRESS);
  EXPECT_EQ(result1, BLOCK_TYPE(DCP_OPT_IP, DCP_SUB_OPT_IP_MAC));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_IP_PARAMETER) {
  uint16_t result2 = dcp_option_bit_offset(DCP_BIT_IDX_IP_PARAMETER);
  EXPECT_EQ(result2, BLOCK_TYPE(DCP_OPT_IP, DCP_SUB_OPT_IP_PARAM));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_IP_FULL_IP_SUITE) {
  uint16_t result3 = dcp_option_bit_offset(DCP_BIT_IDX_IP_FULL_IP_SUITE);
  EXPECT_EQ(result3, BLOCK_TYPE(DCP_OPT_IP, DCP_SUB_OPT_IP_FULL_SUITE));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_DEV_PROP_NAME_OF_VENDOR) {
  uint16_t result4 = dcp_option_bit_offset(DCP_BIT_IDX_DEV_PROP_NAME_OF_VENDOR);
  EXPECT_EQ(result4, BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION) {
  uint16_t result5 = dcp_option_bit_offset(DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION);
  EXPECT_EQ(result5, BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_DEV_PROP_DEVICE_ID) {
  uint16_t result6 = dcp_option_bit_offset(DCP_BIT_IDX_DEV_PROP_DEVICE_ID);
  EXPECT_EQ(result6, BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ID));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_DEV_PROP_DEVICE_ROLE) {
  uint16_t result7 = dcp_option_bit_offset(DCP_BIT_IDX_DEV_PROP_DEVICE_ROLE);
  EXPECT_EQ(result7, BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_ROLE));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_DEV_PROP_DEVICE_OPTIONS) {
  uint16_t result8 = dcp_option_bit_offset(DCP_BIT_IDX_DEV_PROP_DEVICE_OPTIONS);
  EXPECT_EQ(result8, BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_OPTIONS));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_DEV_PROP_NAME_OF_ALIAS) {
  uint16_t result9 = dcp_option_bit_offset(DCP_BIT_IDX_DEV_PROP_NAME_OF_ALIAS);
  EXPECT_EQ(result9, BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_ALIAS));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_DEV_PROP_DEVICE_INSTANCE) {
  uint16_t result10 = dcp_option_bit_offset(DCP_BIT_IDX_DEV_PROP_DEVICE_INSTANCE);
  EXPECT_EQ(result10, BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_DEVICE_INSTANCE));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_DEV_PROP_OEM_DEVICE_ID) {
  uint16_t result11 = dcp_option_bit_offset(DCP_BIT_IDX_DEV_PROP_OEM_DEVICE_ID);
  EXPECT_EQ(result11, BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_OEM_DEVICE_ID));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_DEV_PROP_STANDARD_GATEWAY) {
  uint16_t result12 = dcp_option_bit_offset(DCP_BIT_IDX_DEV_PROP_STANDARD_GATEWAY);
  EXPECT_EQ(result12, BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_STD_GATEWAY));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_DEV_PROP_RSI_PROPERTIES) {
  uint16_t result13 = dcp_option_bit_offset(DCP_BIT_IDX_DEV_PROP_RSI_PROPERTIES);
  EXPECT_EQ(result13, BLOCK_TYPE(DCP_OPT_DEV_PROP, DCP_SUB_OPT_DEV_PROP_RSI));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_DHCP_CLIENT_IDENT) {
  uint16_t result14 = dcp_option_bit_offset(DCP_BIT_IDX_DHCP_CLIENT_IDENT);
  EXPECT_EQ(result14, BLOCK_TYPE(DCP_OPT_DHCP, DCP_SUB_OPT_DHCP_CLIENT_IDENT));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_CTRL_START) {
  uint16_t result15 = dcp_option_bit_offset(DCP_BIT_IDX_CTRL_START);
  EXPECT_EQ(result15, BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_START));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_CTRL_STOP) {
  uint16_t result16 = dcp_option_bit_offset(DCP_BIT_IDX_CTRL_STOP);
  EXPECT_EQ(result16, BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_STOP));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_CTRL_SIGNAL) {
  uint16_t result17 = dcp_option_bit_offset(DCP_BIT_IDX_CTRL_SIGNAL);
  EXPECT_EQ(result17, BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_SIGNAL));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_CTRL_RESPONSE) {
  uint16_t result18 = dcp_option_bit_offset(DCP_BIT_IDX_CTRL_RESPONSE);
  EXPECT_EQ(result18, BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_RESPONSE));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_CTRL_FACTORY_RESET) {
  uint16_t result19 = dcp_option_bit_offset(DCP_BIT_IDX_CTRL_FACTORY_RESET);
  EXPECT_EQ(result19, BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_FACTORY_RESET));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_CTRL_RESET_TO_FACTORY) {
  uint16_t result20 = dcp_option_bit_offset(DCP_BIT_IDX_CTRL_RESET_TO_FACTORY);
  EXPECT_EQ(result20, BLOCK_TYPE(DCP_OPT_CONTROL, DCP_SUB_OPT_CTRL_RESET_TO_FACTORY));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_DEVICE_INITIATIVE) {
  uint16_t result21 = dcp_option_bit_offset(DCP_BIT_IDX_DEVICE_INITIATIVE);
  EXPECT_EQ(result21, BLOCK_TYPE(DCP_OPT_DEV_INITIATIVE, DCP_SUB_OPT_DEV_INITIATIVE));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_NME_DOMAIN_NME_DOMAIN) {
  uint16_t result22 = dcp_option_bit_offset(DCP_BIT_IDX_NME_DOMAIN_NME_DOMAIN);
  EXPECT_EQ(result22, BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_DOMAIN));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_NME_DOMAIN_NME_PRIO) {
  uint16_t result23 = dcp_option_bit_offset(DCP_BIT_IDX_NME_DOMAIN_NME_PRIO);
  EXPECT_EQ(result23, BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_PRIORITY));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_NME_DOMAIN_NME_PARAMETER_UUID) {
  uint16_t result24 = dcp_option_bit_offset(DCP_BIT_IDX_NME_DOMAIN_NME_PARAMETER_UUID);
  EXPECT_EQ(result24, BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_PARAM_UUID));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_NME_DOMAIN_NME_NAME) {
  uint16_t result25 = dcp_option_bit_offset(DCP_BIT_IDX_NME_DOMAIN_NME_NAME);
  EXPECT_EQ(result25, BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_NAME));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_NME_DOMAIN_CIM_INTERFACE) {
  uint16_t result26 = dcp_option_bit_offset(DCP_BIT_IDX_NME_DOMAIN_CIM_INTERFACE);
  EXPECT_EQ(result26, BLOCK_TYPE(DCP_OPT_NME_DOMAIN, DCP_SUB_OPT_NME_CIM_INTERFACE));
}

TEST(dcp_option_bit_offset, DCP_BIT_IDX_ALL_SELECTOR) {
  uint16_t result27 = dcp_option_bit_offset(DCP_BIT_IDX_ALL_SELECTOR);
  EXPECT_EQ(result27, BLOCK_TYPE(DCP_OPT_ALL_SELECTOR, DCP_SUB_OPT_ALL_SELECTOR));
}

TEST(dcp_option_bit_offset, unknow) {
  uint16_t result28 = dcp_option_bit_offset(DCP_BIT_IDX_NUM);
  EXPECT_EQ(result28, 0);
}
