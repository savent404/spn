#include <gtest/gtest.h>
#include <spn/db.h>
#include <spn/dcp.h>
#include <spn/errno.h>
#include "test_data.hpp"

#include "ddcp.hpp"

using test::DataParser;
using test::Ddcp;

TEST_F(Ddcp, ident_ind_all_selector) {
  /* functional test from wireshark */
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
  EXPECT_EQ(dcp_srv_ident_ind(&dcp, &dcp.mcr_ctx[0], frame->data(), frame->size()), SPN_OK);
  EXPECT_EQ(dcp.mcr_ctx[0].xid, 0x1000001);
  EXPECT_EQ(dcp.mcr_ctx[0].state, DCP_STATE_IDENT_RES);

  int len = dcp_srv_ident_rsp(&dcp, &dcp.mcr_ctx[0], out, sizeof(out));
  EXPECT_GT(len, 0);
  EXPECT_EQ(dcp.mcr_ctx[0].state, DCP_STATE_IDLE);

  /* Compare with int.resp */
}

TEST_F(Ddcp, ident_ind_syntax_filter) {
  /* NameOfStation can't appear with NameOfAlias */
  DataParser parser;
  auto frame = parser({
      DCP_SRV_ID_IDENT, DCP_SRV_TYPE_REQ,  // srv
      0x00, 0x00, 0x00, 0xEE,              // xid
      0, 1,                                // response delay factor
      0, 16,                               // length
      // block begin
      DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION,  // option
      0, 4,                                                       // length
      'f', 'o', 'o', 'b',                                         // data
                                                                  // block end
      // block begin
      DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_ALIAS,  // option
      0, 4,                                                     // length
      'f', 'o', 'o', 'b',                                       // data
                                                                // block end
  });
  declare_name_of_station("foob");
  declare_name_of_vendor("foob");
  EXPECT_EQ(dcp_srv_ident_ind(&dcp, &dcp.mcr_ctx[0], frame->data(), frame->size()), -SPN_EAGAIN);

  frame = parser({
      DCP_SRV_ID_IDENT, DCP_SRV_TYPE_REQ,  // srv
      0x00, 0x00, 0x00, 0xEE,              // xid
      0, 1,                                // response delay factor
      0, 16,                               // length
      // block begin
      // block begin
      DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_ALIAS,  // option
      0, 4,                                                     // length
      'f', 'o', 'o', 'b',                                       // data
                                                                // block end
      // block begin
      DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION,  // option
      0, 4,                                                       // length
      'f', 'o', 'o', 'b',                                         // data
                                                                  // block end
  });
  EXPECT_EQ(dcp_srv_ident_ind(&dcp, &dcp.mcr_ctx[0], frame->data(), frame->size()), -SPN_EAGAIN);

  frame = parser({
      DCP_SRV_ID_IDENT, DCP_SRV_TYPE_REQ,  // srv
      0x00, 0x00, 0x00, 0xEE,              // xid
      0, 1,                                // response delay factor
      0, 16,                               // length
      // block begin
      DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION,  // option
      0, 4,                                                       // length
      'f', 'o', 'o', 'b',                                         // data
                                                                  // block end
      // block begin
      DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR,  // option
      0, 4,                                                      // length
      'f', 'o', 'o', 'b',                                        // data
                                                                 // block end
  });
  EXPECT_EQ(dcp_srv_ident_ind(&dcp, &dcp.mcr_ctx[0], frame->data(), frame->size()), SPN_OK);

  dcp.mcr_ctx[0].state = DCP_STATE_IDLE;
  frame = parser({
      DCP_SRV_ID_IDENT, DCP_SRV_TYPE_REQ,  // srv
      0x00, 0x00, 0x00, 0xEE,              // xid
      0, 1,                                // response delay factor
      0, 0,                                // length
  });
  EXPECT_EQ(dcp_srv_ident_ind(&dcp, &dcp.mcr_ctx[0], frame->data(), frame->size()), -SPN_EAGAIN);
}
TEST_F(Ddcp, ident_ind_name_of_vendor_but_db_not_set) {
  /**
   * The first option must be name of station
   */
  DataParser parser;
  auto frame = parser({
      DCP_SRV_ID_IDENT, DCP_SRV_TYPE_REQ,  // srv
      0x00, 0x00, 0x00, 0xEE,              // xid
      0, 1,                                // response delay factor
      0, 16,                               // length

      // block begin
      DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION,  // option
      0, 4,                                                       // length
      'f', 'o', 'o', 'b',                                         // data
                                                                  // block end
      // block begin
      DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR,  // option
      0, 4,                                                      // length
      'f', 'b', 'b', 'f',                                        // data
  });
  declare_name_of_station("foob");
  EXPECT_EQ(dcp_srv_ident_ind(&dcp, &dcp.mcr_ctx[0], frame->data(), frame->size()), -SPN_EAGAIN);
}

TEST_F(Ddcp, ident_ind_unknow_option) {
  /**
   * The first option must be name of station
   */
  DataParser parser;
  auto frame = parser({
      DCP_SRV_ID_IDENT, DCP_SRV_TYPE_REQ,  // srv
      0x00, 0x00, 0x00, 0xEE,              // xid
      0, 1,                                // response delay factor
      0, 16,                               // length

      // block begin
      DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION,  // option
      0, 4,                                                       // length
      'f', 'o', 'o', 'b',                                         // data
                                                                  // block end
      // block begin
      0, 0,                // option
      0, 4,                // length
      'f', 'b', 'b', 'f',  // data
  });
  declare_name_of_station("foob");
  EXPECT_EQ(dcp_srv_ident_ind(&dcp, &dcp.mcr_ctx[0], frame->data(), frame->size()), -SPN_EAGAIN);
}

TEST_F(Ddcp, ident_ind_name_of_vendor) {
  /**
   * The first option must be name of station
   */
  DataParser parser;
  auto frame = parser({
      DCP_SRV_ID_IDENT, DCP_SRV_TYPE_REQ,  // srv
      0x00, 0x00, 0x00, 0xEE,              // xid
      0, 1,                                // response delay factor
      0, 16,                               // length

      // block begin
      DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION,  // option
      0, 4,                                                       // length
      'f', 'o', 'o', 'b',                                         // data
                                                                  // block end
      // block begin
      DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR,  // option
      0, 4,                                                      // length
      'f', 'b', 'b', 'f',                                        // data
  });
  declare_name_of_station("foob");
  declare_name_of_vendor("fbbf");
  EXPECT_EQ(dcp_srv_ident_ind(&dcp, &dcp.mcr_ctx[0], frame->data(), frame->size()), SPN_OK);
  EXPECT_EQ(dcp.mcr_ctx[0].xid, 0xEE);
  EXPECT_EQ(dcp.mcr_ctx[0].state, DCP_STATE_IDENT_RES);
  dcp.mcr_ctx[0].state = DCP_STATE_IDLE; /* clear state */

  frame = parser({
      DCP_SRV_ID_IDENT, DCP_SRV_TYPE_REQ,  // srv
      0x00, 0x00, 0x00, 0xEE,              // xid
      0, 1,                                // response delay factor
      0, 16,                               // length

      // block begin
      DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION,  // option
      0, 4,                                                       // length
      'f', 'o', 'o', 'b',                                         // data
                                                                  // block end
      // block begin
      DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_VENDOR,  // option
      0, 4,                                                      // length
      'f', 'x', 'x', 'f',                                        // data
  });
  EXPECT_EQ(dcp_srv_ident_ind(&dcp, &dcp.mcr_ctx[0], frame->data(), frame->size()), -SPN_EAGAIN);
}

TEST_F(Ddcp, ident_ind_name_of_station) {
  /**
   * Make sure that ident.ind handle db's statis string and dynamic string in a right way
   */
  DataParser parser;
  auto frame = parser({
      DCP_SRV_ID_IDENT, DCP_SRV_TYPE_REQ,  // srv
      0x00, 0x00, 0x00, 0xEE,              // xid
      0, 1,                                // response delay factor
      0, 8,                                // length

      // block begin
      DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION,  // option
      0, 4,                                                       // length
      'f', 'o', 'o', 'b',                                         // data
                                                                  // block end
  });
  declare_name_of_station("foob");
  EXPECT_EQ(dcp_srv_ident_ind(&dcp, &dcp.mcr_ctx[0], frame->data(), frame->size()), SPN_OK);
  EXPECT_EQ(dcp.mcr_ctx[0].xid, 0xEE);
  EXPECT_EQ(dcp.mcr_ctx[0].state, DCP_STATE_IDENT_RES);
  dcp.mcr_ctx[0].state = DCP_STATE_IDLE; /* clear state */
}

TEST_F(Ddcp, ident_ind_name_of_station_dyn) {
  /**
   * Make sure that ident.ind handle db's statis string and dynamic string in a right way
   */
  DataParser parser;
  auto frame = parser({
      DCP_SRV_ID_IDENT, DCP_SRV_TYPE_REQ,  // srv
      0x00, 0x00, 0x00, 0xEE,              // xid
      0, 1,                                // response delay factor
      0, 14,                               // length

      // block begin
      DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION,  // option
      0, 10,                                                      // length
      'f', 'o', 'o', 'b',                                         // data
      'b', 'o', 'o', 'f', 't', 't',
      // block end
  });
  declare_name_of_station("foobbooftt");
  EXPECT_EQ(dcp_srv_ident_ind(&dcp, &dcp.mcr_ctx[0], frame->data(), frame->size()), SPN_OK);
  EXPECT_EQ(dcp.mcr_ctx[0].xid, 0xEE);
  EXPECT_EQ(dcp.mcr_ctx[0].state, DCP_STATE_IDENT_RES);
}

TEST_F(Ddcp, ident_ind_name_of_station_not_match) {
  /* Additional test for name of station */
  DataParser parser;
  auto frame = parser({
      DCP_SRV_ID_IDENT, DCP_SRV_TYPE_REQ,  // srv
      0x00, 0x00, 0x00, 0xEE,              // xid
      0, 1,                                // response delay factor
      0, 8,                                // length

      // block begin
      DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION,  // option
      0, 4,                                                       // length
      'f', 'o', 'o', 'b',                                         // data
                                                                  // block end
  });
  declare_name_of_station("foob-ver-long");
  EXPECT_EQ(dcp_srv_ident_ind(&dcp, &dcp.mcr_ctx[0], frame->data(), frame->size()), -SPN_EAGAIN);
}

TEST_F(Ddcp, ident_ind_name_of_station_dyn_name_with_invalid_length) {
  DataParser parser;
  auto frame = parser({
      DCP_SRV_ID_IDENT, DCP_SRV_TYPE_REQ,  // srv
      0x00, 0x00, 0x00, 0xEE,              // xid
      0, 1,                                // response delay factor
      0, 8,                                // length

      // block begin
      DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION,  // option
      0, 10,                                                      // length
      'f', 'o', 'o', 'b',                                         // data
      'b', 'o', 'o', 'f', 't', 't',
      // block end
  });
  declare_name_of_station("foobbooftt");
  EXPECT_NE(dcp_srv_ident_ind(&dcp, &dcp.mcr_ctx[0], frame->data(), frame->size()), SPN_OK);
}

TEST_F(Ddcp, ident_ind_craped_frame) {
  /**
   * ident.req can't receive the frame that has less that dcp header tell us
   */
  DataParser parser;
  auto frame = parser({
      DCP_SRV_ID_IDENT, DCP_SRV_TYPE_REQ,  // srv
      0x00, 0x00, 0x00, 0xEE,              // xid
      0, 1,                                // response delay factor
      0, 8,                                // length

      // block begin
      DCP_OPTION_DEV_PROP, DCP_SUB_OPT_DEV_PROP_NAME_OF_STATION,  // option
      0, 4,                                                       // length
      'f', 'o', 'o', 'b',                                         // data
                                                                  // block end
  });

  declare_name_of_station("foob");
  EXPECT_EQ(dcp_srv_ident_ind(&dcp, &dcp.mcr_ctx[0], frame->data(), 10), -SPN_EMSGSIZE);
}

TEST_F(Ddcp, ident_ind_invalid_requestion) {
  DataParser parser;
  auto frame = parser({
      DCP_SRV_ID_IDENT, DCP_SRV_TYPE_REQ,  // srv
      0x00, 0x00, 0x00, 0xEE,              // xid
      0, 1,                                // response delay factor
      0, 8,                                // length

      // block begin
  });
}

TEST_F(Ddcp, ident_cnf_invalid) {
  DataParser parser;
  auto frame = parser(test_data::dcp::kDcpIdentRespX208);

  /** assume that new interface is registered in 0x1000 */
  dcp.cnf_interface_id = 0x1000;
  dcp.cnf_xid = 0xBEEF;

  /* drop first 16 bytes */
  frame->erase(frame->begin(), frame->begin() + 16);
  EXPECT_EQ(dcp_srv_ident_cnf(&dcp, frame->data(), frame->size()), -SPN_ENXIO);
}

TEST_F(Ddcp, ident_cnf_ecopn) {
  DataParser parser;
  auto frame = parser(test_data::dcp::kDcpIdentRespEcoPn);

  /** assume that new interface is registered in 0x1000 */
  dcp.cnf_interface_id = 0x1000;
  dcp.cnf_xid = 0x0001'94EF;

  /* drop first 16 bytes */
  frame->erase(frame->begin(), frame->begin() + 16);
  EXPECT_EQ(dcp_srv_ident_cnf(&dcp, frame->data(), frame->size()), SPN_OK);

  struct db_interface* intf;
  struct db_object* obj;
  EXPECT_EQ(db_get_interface(&db, 0x1000, &intf), SPN_OK);

  EXPECT_EQ(db_get_object(&intf->objects, db_id_t::DB_ID_NAME_OF_STATION, &obj), SPN_OK);
  EXPECT_STREQ((char*)obj->data.ptr, "et200ecopn.dev7");
  EXPECT_EQ(db_get_object(&intf->objects, db_id_t::DB_ID_NAME_OF_VENDOR, &obj), SPN_OK);
  EXPECT_STREQ((char*)obj->data.ptr, "ET200ecoPN");
  EXPECT_EQ(db_get_object(&intf->objects, db_id_t::DB_ID_DEVICE_ROLE, &obj), SPN_OK);
  EXPECT_EQ(obj->data.u8, 0x01);
  EXPECT_EQ(db_get_object(&intf->objects, db_id_t::DB_ID_VENDOR_ID, &obj), SPN_OK);
  EXPECT_EQ(obj->data.u16, 0x002a);
  EXPECT_EQ(db_get_object(&intf->objects, db_id_t::DB_ID_DEVICE_ID, &obj), SPN_OK);
  EXPECT_EQ(obj->data.u16, 0x0306);
  EXPECT_EQ(db_get_object(&intf->objects, db_id_t::DB_ID_DEVICE_INSTANCE, &obj), SPN_OK);
  EXPECT_EQ(obj->data.u16, 0x0001);
  EXPECT_EQ(db_get_object(&intf->objects, db_id_t::DB_ID_DEVICE_OPTIONS, &obj), SPN_OK);
  EXPECT_EQ(obj->header.len, 4);
  EXPECT_EQ(obj->data.u32, 0x1FB);
}

TEST_F(Ddcp, set_ind_name_of_station) {
  DataParser parser;
  struct db_object* obj;
  struct dcp_ucr_ctx ucr;
  auto frame = parser(test_data::dcp::kDcpNameOfStationSetReq);

  declare_name_of_station("station_xxx");

  frame->erase(frame->begin(), frame->begin() + 16);
  EXPECT_EQ(dcp_srv_set_ind(&dcp, &ucr, frame->data(), frame->size()), SPN_OK);
  EXPECT_EQ(ucr.req_options_bitmap, 1 << DCP_BITMAP_DEVICE_PROPERTIES_NAME_OF_STATION | 1 << DCP_BITMAP_CONTROL_STOP);
  EXPECT_EQ(db_get_interface_object(&db, 0, db_id_t::DB_ID_NAME_OF_STATION, &obj), SPN_OK);
  EXPECT_STRNE((char*)obj->data.ptr, "station_xxx");
}

TEST_F(Ddcp, set_ind_ip_param) {
  DataParser parser;
  struct db_object* obj;
  struct dcp_ucr_ctx ucr;
  auto frame = parser(test_data::dcp::kDcpIpParamSetReq);

  declare_ip_param(0, 0, 0);

  frame->erase(frame->begin(), frame->begin() + 16);
  EXPECT_EQ(dcp_srv_set_ind(&dcp, &ucr, frame->data(), frame->size()), SPN_OK);

  EXPECT_EQ(db_get_interface_object(&db, 0, db_id_t::DB_ID_IP_ADDR, &obj), SPN_OK);
  EXPECT_NE(obj->data.u32, 0);

  EXPECT_EQ(db_get_interface_object(&db, 0, db_id_t::DB_ID_IP_MASK, &obj), SPN_OK);
  EXPECT_NE(obj->data.u32, 0);

  EXPECT_EQ(db_get_interface_object(&db, 0, db_id_t::DB_ID_IP_GATEWAY, &obj), SPN_OK);
  EXPECT_EQ(obj->data.u32, 0);
}

TEST_F(Ddcp, set_ind_station_of_name) {
  DataParser parser;
  struct db_object* obj;
  struct dcp_ucr_ctx ucr;
  auto frame = parser(test_data::dcp::kDcpNameOfStationSetReq);

  declare_name_of_station("station");

  frame->erase(frame->begin(), frame->begin() + 16);
  EXPECT_EQ(dcp_srv_set_ind(&dcp, &ucr, frame->data(), frame->size()), SPN_OK);

  EXPECT_EQ(db_get_interface_object(&db, 0, db_id_t::DB_ID_NAME_OF_STATION, &obj), SPN_OK);
  EXPECT_STRNE((char*)obj->data.ptr, "station");
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
  EXPECT_EQ(dcp_srv_set_ind(&dcp, &ucr, frame->data(), frame->size()), SPN_OK);

  EXPECT_EQ(db_get_interface_object(&db, 0, db_id_t::DB_ID_IP_ADDR, &obj), SPN_OK);
  EXPECT_NE(obj->data.u32, 0);

  EXPECT_EQ(db_get_interface_object(&db, 0, db_id_t::DB_ID_IP_MASK, &obj), SPN_OK);
  EXPECT_NE(obj->data.u32, 0);

  EXPECT_EQ(db_get_interface_object(&db, 0, db_id_t::DB_ID_IP_GATEWAY, &obj), SPN_OK);
  EXPECT_EQ(obj->data.u32, 0);

  frame = parser(test_data::dcp::kDcpIpParamSetResp);
  frame->erase(frame->begin(), frame->begin() + 16);
  EXPECT_GE(dcp_srv_set_rsp(&dcp, &ucr, out, sizeof(out)), frame->size());

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
  EXPECT_EQ(dcp_srv_set_ind(&dcp, &ucr, frame->data(), frame->size()), SPN_OK);

  EXPECT_EQ(db_get_interface_object(&db, 0, db_id_t::DB_ID_NAME_OF_STATION, &obj), SPN_OK);
  EXPECT_STRNE((char*)obj->data.ptr, "station");

  frame = parser(test_data::dcp::kDcpNameOfStationSetResp);
  frame->erase(frame->begin(), frame->begin() + 16);
  EXPECT_GE(dcp_srv_set_rsp(&dcp, &ucr, out, sizeof(out)), frame->size());

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
  EXPECT_EQ(dcp_srv_get_ind(&dcp, &ucr, frame->data(), frame->size()), SPN_OK);
  EXPECT_EQ(ucr.req_options_bitmap, 1 << DCP_BITMAP_IP_PARAMETER | 1 << DCP_BITMAP_DHCP_CLIENT_IDENT);
}

TEST_F(Ddcp, get_rsp_simple) {
  DataParser parser;
  db_value_t val;
  struct dcp_ucr_ctx ucr;
  char out[1500];

  memset(out, 0x5A, sizeof(out));

  auto f_req = parser(test_data::dcp::kDcpGetReqSample1);
  auto f_rsp = parser(test_data::dcp::kDcpGetRspSample1);

  f_req->erase(f_req->begin(), f_req->begin() + 16);
  f_rsp->erase(f_rsp->begin(), f_rsp->begin() + 16);

  EXPECT_EQ(dcp_srv_get_ind(&dcp, &ucr, f_req->data(), f_req->size()), SPN_OK);
  EXPECT_EQ(ucr.req_options_bitmap, 1 << DCP_BITMAP_IP_PARAMETER | 1 << DCP_BITMAP_DHCP_CLIENT_IDENT);
  EXPECT_EQ(ucr.xid, 0x0f020013);

  /* prepare db */
  db_add_interface(&db, 0);

  val.u16 = 1; /* static ip */
  db_add_object(&db.interfaces[0].objects, db_id_t::DB_ID_IP_BLOCK_INFO, 0, 0, 0, &val);

  val.u32 = 0xc0a802bc; /* 192.168.2.188 */
  db_add_object(&db.interfaces[0].objects, db_id_t::DB_ID_IP_ADDR, 0, 0, 0, &val);

  val.u32 = 0xffffff00; /* 255.255.255.0 */
  db_add_object(&db.interfaces[0].objects, db_id_t::DB_ID_IP_MASK, 0, 0, 0, &val);

  val.u32 = 0xc0a802bc; /* 192.168.2.188 */
  db_add_object(&db.interfaces[0].objects, db_id_t::DB_ID_IP_GATEWAY, 0, 0, 0, &val);

  EXPECT_GE(dcp_srv_get_rsp(&dcp, &ucr, (uint8_t*)out, sizeof(out)), f_rsp->size());
  for (int i = 0; i < f_rsp->size(); i++) {
    uint8_t a = out[i], b = f_rsp->at(i);
    EXPECT_EQ(a, b) << "mismatch at " << i;
  }
}
