#include <gtest/gtest.h>
#include <memory>
#include "dcp_ctx.hpp"

namespace {

using test::dcp_inst_ptr;
using test::dcp_instance;

struct DcpPatner : public ::testing::Test {
  dcp_inst_ptr controller;
  dcp_inst_ptr device;

  void SetUp() override {
    controller = std::make_shared<dcp_instance>();
    device = std::make_shared<dcp_instance>();
    controller->db_setup();
    device->db_setup();
    device->db_info_setup(0x0a000001, {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, "device", "iod", 0x1234, 0x5678, 0x01);
    controller->db_info_setup(0x0a000002, {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}, "controller", "ioc", 0x1234, 0x5678,
                              0x02);
  }

  void TearDown() override {
    controller.reset();
    device.reset();
  }
};

void tain_buffer(char* buf, uint16_t len) {
  for (uint16_t i = 0; i < len; i++) {
    buf[i] = i;
  }
}

}  // namespace

TEST_F(DcpPatner, ident_all) {
  static char buf[1500];
  uint16_t buf_len;

  // ident all
  tain_buffer(buf, 1500);
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_ALL_SELECTOR;
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);

  struct dcp_mcr_ctx mcr;
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), SPN_OK);

  tain_buffer(buf, 1500);
  ASSERT_EQ(dcp_srv_ident_rsp(&device->dcp, &mcr, buf, &buf_len), SPN_OK);

  ASSERT_EQ(dcp_srv_ident_cnf(&controller->dcp, &controller->dcp.mcs_ctx, buf + 2, buf_len - 2, 0), SPN_OK);
}

TEST_F(DcpPatner, ident_station_name) {
  static char buf[1500];
  uint16_t buf_len;

  tain_buffer(buf, 1500);
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION;
  controller->dcp.mcs_ctx.station_name = "device";
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);

  struct dcp_mcr_ctx mcr;
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), SPN_OK);

  tain_buffer(buf, 1500);
  ASSERT_EQ(dcp_srv_ident_rsp(&device->dcp, &mcr, buf, &buf_len), SPN_OK);

  ASSERT_EQ(dcp_srv_ident_cnf(&controller->dcp, &controller->dcp.mcs_ctx, buf + 2, buf_len - 2, 0), SPN_OK);
}

TEST_F(DcpPatner, ident_alias_name) {
  static char buf[1500];
  uint16_t buf_len;

  tain_buffer(buf, 1500);
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_ALIAS;
  controller->dcp.mcs_ctx.alias_name = "port-001.device";
  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);

  struct dcp_mcr_ctx mcr;
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), SPN_OK);

  tain_buffer(buf, 1500);
  ASSERT_EQ(dcp_srv_ident_rsp(&device->dcp, &mcr, buf, &buf_len), SPN_OK);

  ASSERT_EQ(dcp_srv_ident_cnf(&controller->dcp, &controller->dcp.mcs_ctx, buf + 2, buf_len - 2, 0), SPN_OK);
}

TEST_F(DcpPatner, ident_multiple) {
  static char buf[1500];
  uint16_t buf_len;

  tain_buffer(buf, 1500);
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_ALIAS;
  controller->dcp.mcs_ctx.alias_name = "port-001.device";

  controller->dcp.mcs_ctx.req_options_bitmap |= 1 << DCP_BIT_IDX_IP_PARAMETER;
  controller->dcp.mcs_ctx.ip_addr = 0x0a000001;
  controller->dcp.mcs_ctx.ip_mask = 0x00FFFFFF;
  controller->dcp.mcs_ctx.ip_gw = 0;

  controller->dcp.mcs_ctx.req_options_bitmap |= 1 << DCP_BIT_IDX_DEV_PROP_DEVICE_ID;
  controller->dcp.mcs_ctx.device_id = 0x1234;
  controller->dcp.mcs_ctx.vendor_id = 0x5678;

  ASSERT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);

  struct dcp_mcr_ctx mcr;
  ASSERT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), SPN_OK);

  tain_buffer(buf, 1500);
  ASSERT_EQ(dcp_srv_ident_rsp(&device->dcp, &mcr, buf, &buf_len), SPN_OK);

  ASSERT_EQ(dcp_srv_ident_cnf(&controller->dcp, &controller->dcp.mcs_ctx, buf + 2, buf_len - 2, 0), SPN_OK);
}

TEST_F(DcpPatner, unspported_filter) {}
