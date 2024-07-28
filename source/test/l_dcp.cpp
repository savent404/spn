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

  // setup device
  device->db_info_setup(0x0a000001, {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, "device", "iod", 0x1234, 0x5678, 0x01);
  // setup controller
  controller->db_info_setup(0x0a000002, {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}, "controller", "ioc", 0x1234, 0x5678,
                            0x02);

  // ident all
  tain_buffer(buf, 1500);
  controller->dcp.mcs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_ALL_SELECTOR;
  EXPECT_EQ(dcp_srv_ident_req(&controller->dcp, &controller->dcp.mcs_ctx, buf, &buf_len), SPN_OK);

  struct dcp_mcr_ctx mcr;
  EXPECT_EQ(dcp_srv_ident_ind(&device->dcp, &mcr, buf + 2, buf_len - 2), SPN_OK);

  tain_buffer(buf, 1500);
  EXPECT_EQ(dcp_srv_ident_rsp(&device->dcp, &mcr, buf, &buf_len), SPN_OK);

  EXPECT_EQ(dcp_srv_ident_cnf(&controller->dcp, &controller->dcp.mcs_ctx, buf + 2, buf_len - 2, 0), SPN_OK);
}

TEST_F(DcpPatner, unspported_filter) {}
