#include <gtest/gtest.h>
#include <spn/db.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include "test_data.hpp"

namespace {

using DataParser = test_data::dcp::DataParser;

struct Ddcp : public ::testing::Test {
  struct dcp_ctx dcp;
  struct db_ctx db;

  explicit Ddcp() {
    memset(&dcp, 0, sizeof(dcp));
    memset(&db, 0, sizeof(db));
  }

  void SetUp() override {
    struct db_interface* iface;

    db_init(&db);
    dcp_init(&dcp, &db);

    db_add_interface(&db, 0);
    db_get_interface(&db, 0, &iface);
    db_add_port(iface, 0);
  }

  void TearDown() override {
    dcp_deinit(&dcp);
    db_deinit(&db);
  }

  void declare_block_info(enum ip_block_info info) {
    db_value_t value;
    struct db_interface* iface;
    value.u16 = info;
    db_get_interface(&db, 0, &iface);
    db_add_object(&iface->objects, db_id_t::DB_ID_IP_BLOCK_INFO, 0, 0, 0, &value);
  }

  void declare_ip_param(uint32_t ip, uint32_t mask, uint32_t gw) {
    db_value_t value;
    struct db_interface* iface;
    db_get_interface(&db, 0, &iface);
    value.u32 = ip;
    db_add_object(&iface->objects, db_id_t::DB_ID_IP_ADDR, 0, 0, 0, &value);
    value.u32 = mask;
    db_add_object(&iface->objects, db_id_t::DB_ID_IP_MASK, 0, 0, 0, &value);
    value.u32 = gw;
    db_add_object(&iface->objects, db_id_t::DB_ID_IP_GATEWAY, 0, 0, 0, &value);
  }

  void declare_name_of_station(const std::string name) {
    db_value_t value;
    struct db_interface* iface;
    value.ptr = strdup(name.c_str());
    db_get_interface(&db, 0, &iface);
    db_add_object(&iface->objects, db_id_t::DB_ID_NAME_OF_STATION, 1, 1, strlen((char*)value.ptr), &value);
  }

  void declare_name_of_vendor(const std::string name) {
    db_value_t value;
    struct db_interface* iface;
    value.ptr = strdup(name.c_str());
    db_get_interface(&db, 0, &iface);
    db_add_object(&iface->objects, db_id_t::DB_ID_NAME_OF_VENDOR, 1, 1, strlen((char*)value.ptr), &value);
  }

  void declare_device_id(uint16_t vendor_id, uint16_t device_id) {
    db_value_t value;
    struct db_interface* iface;

    db_get_interface(&db, 0, &iface);
    value.u16 = vendor_id;
    db_add_object(&iface->objects, db_id_t::DB_ID_VENDOR_ID, 0, 0, 0, &value);
    value.u16 = device_id;
    db_add_object(&iface->objects, db_id_t::DB_ID_DEVICE_ID, 0, 0, 0, &value);
  }

  void declare_device_role(uint16_t role) {
    db_value_t value;
    struct db_interface* iface;
    db_get_interface(&db, 0, &iface);
    value.u16 = role;
    db_add_object(&iface->objects, db_id_t::DB_ID_DEVICE_ROLE, 0, 0, 0, &value);
  }
};

}  // namespace

TEST_F(Ddcp, ident_ind_all_selector) {
  uint8_t out[1500];
  DataParser parser;
  auto frame = parser(test_data::dcp::kDcpAllSelector);

  memset(out, 0xFF, sizeof(out));

  declare_block_info(ip_block_info::IP_BLOCK_INFO_STATIC);
  declare_ip_param(0x0a0a0a0a, 0xffffff00, 0x0a0a0a01);
  declare_name_of_station("station");
  declare_name_of_vendor("vendor");
  declare_device_id(0x1234, 0x5678);
  declare_device_role(1 << device_role::DEV_ROLE_DEVICE_BIT);
  /* drop first 16 bytes */
  frame->erase(frame->begin(), frame->begin() + 16);
  ASSERT_EQ(dcp_srv_ident_ind(&dcp, frame->data(), frame->size()), SPN_OK);
  ASSERT_EQ(dcp.mcr_ctx[0].xid, 0x1000001);
  ASSERT_EQ(dcp.mcr_ctx[0].state, DCP_STATE_IDENT_RES);

  int len = dcp_srv_ident_rsp(&dcp, &dcp.mcr_ctx[0], out, sizeof(out));
  ASSERT_GT(len, 0);
  ASSERT_EQ(dcp.mcr_ctx[0].state, DCP_STATE_IDLE);

  /* Compare with int.resp */
}

TEST_F(Ddcp, ident_cnf_invalid) {
  DataParser parser;
  auto frame = parser(test_data::dcp::kDcpIdentRespX208);

  /** assume that new interface is registered in 0x1000 */
  dcp.cnf_interface_id = 0x1000;
  dcp.cnf_xid = 0xBEEF;

  /* drop first 16 bytes */
  frame->erase(frame->begin(), frame->begin() + 16);
  ASSERT_EQ(dcp_srv_ident_cnf(&dcp, frame->data(), frame->size()), -SPN_ENXIO);
}

TEST_F(Ddcp, ident_cnf_ecopn) {
  DataParser parser;
  auto frame = parser(test_data::dcp::kDcpIdentRespEcoPn);

  /** assume that new interface is registered in 0x1000 */
  dcp.cnf_interface_id = 0x1000;
  dcp.cnf_xid = 0x0001'94EF;

  /* drop first 16 bytes */
  frame->erase(frame->begin(), frame->begin() + 16);
  ASSERT_EQ(dcp_srv_ident_cnf(&dcp, frame->data(), frame->size()), SPN_OK);

  struct db_interface* intf;
  struct db_object* obj;
  ASSERT_EQ(db_get_interface(&db, 0x1000, &intf), SPN_OK);

  ASSERT_EQ(db_get_object(&intf->objects, db_id_t::DB_ID_NAME_OF_STATION, &obj), SPN_OK);
  ASSERT_STREQ((char*)obj->data.ptr, "et200ecopn.dev7");
  ASSERT_EQ(db_get_object(&intf->objects, db_id_t::DB_ID_NAME_OF_VENDOR, &obj), SPN_OK);
  ASSERT_STREQ((char*)obj->data.ptr, "ET200ecoPN");
  ASSERT_EQ(db_get_object(&intf->objects, db_id_t::DB_ID_DEVICE_ROLE, &obj), SPN_OK);
  ASSERT_EQ(obj->data.u8, 0x01);
  ASSERT_EQ(db_get_object(&intf->objects, db_id_t::DB_ID_VENDOR_ID, &obj), SPN_OK);
  ASSERT_EQ(obj->data.u16, 0x002a);
  ASSERT_EQ(db_get_object(&intf->objects, db_id_t::DB_ID_DEVICE_ID, &obj), SPN_OK);
  ASSERT_EQ(obj->data.u16, 0x0306);
  ASSERT_EQ(db_get_object(&intf->objects, db_id_t::DB_ID_DEVICE_INSTANCE, &obj), SPN_OK);
  ASSERT_EQ(obj->data.u16, 0x0001);
  ASSERT_EQ(db_get_object(&intf->objects, db_id_t::DB_ID_DEVICE_OPTIONS, &obj), SPN_OK);
  ASSERT_EQ(obj->header.len, 4);
  ASSERT_EQ(obj->data.u32, 0x1FB);
}

TEST_F(Ddcp, set_ind_name_of_station) {
  DataParser parser;
  struct db_object* obj;
  struct dcp_ucr_ctx ucr;
  auto frame = parser(test_data::dcp::kDcpNameOfStationSetReq);

  declare_name_of_station("station");

  frame->erase(frame->begin(), frame->begin() + 16);
  ASSERT_EQ(dcp_srv_set_ind(&dcp, &ucr, frame->data(), frame->size()), SPN_OK);
  ASSERT_EQ(ucr.req_options_bitmap, 1 << DCP_BITMAP_DEVICE_PROPERTIES_NAME_OF_STATION | 1 << DCP_BITMAP_CONTROL_STOP);
  ASSERT_EQ(db_get_interface_object(&db, 0, db_id_t::DB_ID_NAME_OF_STATION, &obj), SPN_OK);
  ASSERT_STRNE((char*)obj->data.ptr, "station");
}

TEST_F(Ddcp, set_ind_ip_param) {
  DataParser parser;
  struct db_object* obj;
  struct dcp_ucr_ctx ucr;
  auto frame = parser(test_data::dcp::kDcpIpParamSetReq);

  declare_ip_param(0, 0, 0);

  frame->erase(frame->begin(), frame->begin() + 16);
  ASSERT_EQ(dcp_srv_set_ind(&dcp, &ucr, frame->data(), frame->size()), SPN_OK);

  ASSERT_EQ(db_get_interface_object(&db, 0, db_id_t::DB_ID_IP_ADDR, &obj), SPN_OK);
  ASSERT_NE(obj->data.u32, 0);

  ASSERT_EQ(db_get_interface_object(&db, 0, db_id_t::DB_ID_IP_MASK, &obj), SPN_OK);
  ASSERT_NE(obj->data.u32, 0);

  ASSERT_EQ(db_get_interface_object(&db, 0, db_id_t::DB_ID_IP_GATEWAY, &obj), SPN_OK);
  ASSERT_EQ(obj->data.u32, 0);
}

TEST_F(Ddcp, set_ind_station_of_name) {
  DataParser parser;
  struct db_object* obj;
  struct dcp_ucr_ctx ucr;
  auto frame = parser(test_data::dcp::kDcpNameOfStationSetReq);

  declare_name_of_station("station");

  frame->erase(frame->begin(), frame->begin() + 16);
  ASSERT_EQ(dcp_srv_set_ind(&dcp, &ucr, frame->data(), frame->size()), SPN_OK);

  ASSERT_EQ(db_get_interface_object(&db, 0, db_id_t::DB_ID_NAME_OF_STATION, &obj), SPN_OK);
  ASSERT_STRNE((char*)obj->data.ptr, "station");
}

TEST_F(Ddcp, set_rsp_ip_param) {
  DataParser parser;
  struct db_object* obj;
  struct dcp_ucr_ctx ucr;
  uint8_t out[1500];
  auto frame = parser(test_data::dcp::kDcpIpParamSetReq);

  memset(out, 0xFF, sizeof(out));

  declare_ip_param(0, 0, 0);

  frame->erase(frame->begin(), frame->begin() + 16);
  ASSERT_EQ(dcp_srv_set_ind(&dcp, &ucr, frame->data(), frame->size()), SPN_OK);

  ASSERT_EQ(db_get_interface_object(&db, 0, db_id_t::DB_ID_IP_ADDR, &obj), SPN_OK);
  ASSERT_NE(obj->data.u32, 0);

  ASSERT_EQ(db_get_interface_object(&db, 0, db_id_t::DB_ID_IP_MASK, &obj), SPN_OK);
  ASSERT_NE(obj->data.u32, 0);

  ASSERT_EQ(db_get_interface_object(&db, 0, db_id_t::DB_ID_IP_GATEWAY, &obj), SPN_OK);
  ASSERT_EQ(obj->data.u32, 0);

  frame = parser(test_data::dcp::kDcpIpParamSetResp);
  frame->erase(frame->begin(), frame->begin() + 16);
  ASSERT_GE(dcp_srv_set_rsp(&dcp, &ucr, out, sizeof(out)), frame->size());

  for (int i = 0; i < frame->size(); i++) {
    EXPECT_EQ(out[i], frame->at(i));
    if (out[i] != frame->at(i)) {
      printf("mismatch at %d\n", i);
    }
  }
}

TEST_F(Ddcp, set_rsp_name_of_station) {
  DataParser parser;
  struct db_object* obj;
  struct dcp_ucr_ctx ucr;
  uint8_t out[1500];
  auto frame = parser(test_data::dcp::kDcpNameOfStationSetReq);

  memset(out, 0xFF, sizeof(out));

  declare_name_of_station("station");

  frame->erase(frame->begin(), frame->begin() + 16);
  ASSERT_EQ(dcp_srv_set_ind(&dcp, &ucr, frame->data(), frame->size()), SPN_OK);

  ASSERT_EQ(db_get_interface_object(&db, 0, db_id_t::DB_ID_NAME_OF_STATION, &obj), SPN_OK);
  ASSERT_STRNE((char*)obj->data.ptr, "station");

  frame = parser(test_data::dcp::kDcpNameOfStationSetResp);
  frame->erase(frame->begin(), frame->begin() + 16);
  ASSERT_GE(dcp_srv_set_rsp(&dcp, &ucr, out, sizeof(out)), frame->size());

  for (int i = 0; i < frame->size(); i++) {
    EXPECT_EQ(out[i], frame->at(i));
    if (out[i] != frame->at(i)) {
      printf("mismatch at %d\n", i);
    }
  }
}

TEST_F(Ddcp, get_ind_simple) {
  DataParser parser;
  struct dcp_ucr_ctx ucr;
  auto frame = parser(test_data::dcp::kDcpGetReqSample1);

  frame->erase(frame->begin(), frame->begin() + 16);
  ASSERT_EQ(dcp_srv_get_ind(&dcp, &ucr, frame->data(), frame->size()), SPN_OK);
  ASSERT_EQ(ucr.req_options_bitmap, 1 << DCP_BITMAP_IP_PARAMETER | 1 << DCP_BITMAP_DHCP_CLIENT_IDENT);
}
