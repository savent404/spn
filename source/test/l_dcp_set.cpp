#include <gtest/gtest.h>
#include <spn/dcp.h>
#include "dcp_ctx.hpp"

namespace {

using test::dcp_inst_ptr;
using test::dcp_instance;

struct DcpSet : public ::testing::Test {
  dcp_inst_ptr controller;
  dcp_inst_ptr device;

  void SetUp() override {
    controller = std::make_shared<dcp_instance>();
    device = std::make_shared<dcp_instance>();
    controller->db_setup();
    device->db_setup();
    device->db_info_setup(0, 0x0a000001, {0x00, 0x00, 0x00, 0x00, 0x00, 0x01}, "device", "iod", 0x1234, 0x5678, 0x01);
    controller->db_info_setup(0, 0x0a000002, {0x00, 0x00, 0x00, 0x00, 0x00, 0x02}, "controller", "ioc", 0x1234, 0x5678,
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

TEST_F(DcpSet, SetNameOfStation) {
  static char buf[1500];
  uint16_t buf_len;
  struct dcp_ucr_ctx ucr;

  tain_buffer(buf, sizeof(buf));
  controller->dcp.ucs_ctx.req_options_bitmap = 1 << DCP_BIT_IDX_DEV_PROP_NAME_OF_STATION;
  controller->dcp.ucs_ctx.station_name = "fooboo";
  ASSERT_EQ(dcp_srv_set_req(&controller->dcp, &controller->dcp.ucs_ctx, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_set_ind(&device->dcp, &ucr, buf + 2, buf_len - 2), SPN_OK);
  tain_buffer(buf, sizeof(buf));
  ASSERT_EQ(dcp_srv_set_rsp(&controller->dcp, &ucr, buf, &buf_len), SPN_OK);
  ASSERT_EQ(dcp_srv_set_cnf(&device->dcp, &controller->dcp.ucs_ctx, buf + 2, buf_len - 2), SPN_OK);
}
