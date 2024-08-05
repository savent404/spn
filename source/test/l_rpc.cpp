#include <gtest/gtest.h>
#include <spn/errno.h>
#include <spn/rpc.h>
#include <string.h>
#include <unistd.h>
#include <functional>
#include <memory>
#include <vector>

namespace {
using frame_t = std::shared_ptr<std::vector<uint8_t>>;

frame_t make_frame(const char* hex_str) {
  frame_t frame = std::make_shared<std::vector<uint8_t>>();

  if (strlen(hex_str) % 2) {
    printf("Invalid hex string\n");
    return nullptr;
  }
  frame->reserve(strlen(hex_str) / 2);

  for (size_t i = 0; i < strlen(hex_str); i += 2) {
    char byte[3] = {hex_str[i], hex_str[i + 1], 0};
    frame->push_back(strtol(byte, nullptr, 16));
  }
  return frame;
}

struct rpc : public ::testing::Test {
  void SetUp() override {
    rpc_general_req_fn = nullptr;
    udp_out_fn = nullptr;

    rpc_init(&ctx);
    ctx.fn_rpc_req = rpc_req_fn;
    ctx.fn_udp_out = rpc_udp_out;
  }
  void TearDown() override {}

  static void rpc_req_fn(struct rpc_channel* ch, void* payload, unsigned length) {
    rpc_general_req_fn(ch, payload, length);
  }
  static int rpc_udp_out(void* payload, int length, uint32_t remote_ip, uint16_t remote_port, uint16_t host_port) {
    return udp_out_fn(payload, length, remote_ip, remote_port, host_port);
  }

  struct rpc_ctx ctx;
  static std::function<void(struct rpc_channel*, void*, unsigned)> rpc_general_req_fn;
  static std::function<int(void*, int, uint32_t, uint16_t, uint16_t)> udp_out_fn;
};

std::function<void(struct rpc_channel*, void*, unsigned)> rpc::rpc_general_req_fn = nullptr;
std::function<int(void*, int, uint32_t, uint16_t, uint16_t)> rpc::udp_out_fn = nullptr;
}  // namespace

TEST_F(rpc, ntoh_be) {
  const char conn_req[] =
      "0813a9bc90404ce7053526410800450001e727ec00001e11ed47c0a80246c0a8023cc001889401d3b6040400200000000000dea000006c97"
      "11d1827100640119002adea000016c9711d1827100a02442df7d000001e60000101080014ce7053526410000000000000001000000000000"
      "ffffffff017b000000000000017e000001670000017e000000000000016701010045010000016480b757b0bf46a080dff81e784bdb000008"
      "4ce705352641dea000006c9711d1827100640119002a4000001100c88892000f706c63323030736d6172746d617033010200500100000100"
      "01889200000002002880000020000400010000ffffffff00030003c0000000000000000001000000000005000100010000000103e8000100"
      "0180000003000180010004000180020005000001020050010000020002889200000002002880000020000400010000ffffffff00030003c0"
      "0000000000000000010000000000000005000100010000000103e80001000180000002000180010003000180020004010400580100000100"
      "00000000018000060000000005000100000001000000010000010103e8200000010001000100010101800000000f00000000010000010180"
      "0100000f010000000100000101800200000f01000000010000010101030016010000018892000000000001000300000100c000a000";

  auto conn_req_frame = make_frame(conn_req);
  /* remove udp header */
  conn_req_frame->erase(conn_req_frame->begin(), conn_req_frame->begin() + 0x2A);

  struct rpc_hdr* hdr = (struct rpc_hdr*)conn_req_frame->data();
  struct rpc_ndr_data_req* ndr_data = (struct rpc_ndr_data_req*)hdr->ndr_data;

  if (rpc_is_big_endian(hdr)) {
    rpc_hdr_ntoh(hdr);
    rpc_ndr_ntoh(ndr_data, RPC_PKT_TYPE_REQ);
  }
  ASSERT_EQ(hdr->version, 0x04);
  ASSERT_EQ(hdr->packet_type, 0x00);
  ASSERT_EQ(hdr->flag1, 0x20);
  ASSERT_EQ(hdr->flag2, 0x00);
  ASSERT_EQ(hdr->serial_high, 0x00);
  ASSERT_EQ(hdr->serial_low, 0x00);

  rpc_uuid_t object = {.form{
      .data1 = 0xDEA00000,
      .data2 = 0x6C97,
      .data3 = 0x11D1,
      .data4 = {0x82, 0x71, 0x00, 0x64, 0x01, 0x19, 0x00, 0x2A},
  }};
  ASSERT_EQ(memcmp(&hdr->object_uuid, &object, sizeof(rpc_uuid_t)), 0);

  rpc_uuid_t device_interface = {.form{
      .data1 = 0xDEA00001,
      .data2 = 0x6C97,
      .data3 = 0x11D1,
      .data4 = {0x82, 0x71, 0x00, 0xA0, 0x24, 0x42, 0xDF, 0x7D},
  }};
  ASSERT_EQ(memcmp(&hdr->interface_uuid, &device_interface, sizeof(rpc_uuid_t)), 0);

  ASSERT_EQ(hdr->boot_time, 0);
  ASSERT_EQ(hdr->interface_version_major, 0);
  ASSERT_EQ(hdr->interface_version_minor, 1);
  ASSERT_EQ(hdr->seq_numb, 0);
  ASSERT_EQ(hdr->operation_numb, 0);
  ASSERT_EQ(hdr->interface_hint, 0xFFFF);
  ASSERT_EQ(hdr->activity_hint, 0xFFFF);
  ASSERT_EQ(hdr->length_of_body, 379);
  ASSERT_EQ(hdr->frag_numb, 0);
  ASSERT_EQ(hdr->auth_protocol, 0);
  ASSERT_EQ(hdr->serial_low, 0);

  ASSERT_EQ(ndr_data->args_maxium, 382);
  ASSERT_EQ(ndr_data->args_length, 359);
  ASSERT_EQ(ndr_data->maxium_count, 382);
  ASSERT_EQ(ndr_data->offset, 0);
  ASSERT_EQ(ndr_data->maxium_count, 382);
}

TEST_F(rpc, ntoh_le) {
  const char read_implicit_req[] =
      "30b8510fe7c4c87f549d22120800450000c004650000ff1130bcc0a80202c0a802b98894c0c000ac724404002000100000000000a0de976c"
      "d111827100010306002a0100a0de976cd111827100a02442df7d80d22eb6551eb21101000800067368280000000001000000660400000500"
      "ffffffff54000000000040800000400000004080000000000000400000000009003c01000000000000000000000000000000000000000000"
      "0000000000010000f82100008000000000000000000000000000000000000000000000000000";
  auto frame = make_frame(read_implicit_req);
  frame->erase(frame->begin(), frame->begin() + 0x2A);

  struct rpc_hdr* hdr = (struct rpc_hdr*)frame->data();

  ASSERT_EQ(rpc_is_big_endian(hdr), 0);
  rpc_hdr_ntoh(hdr);

  ASSERT_EQ(hdr->version, 0x04);
  ASSERT_EQ(hdr->packet_type, 0x00);
  ASSERT_EQ(hdr->flag1, 0x20);
  ASSERT_EQ(hdr->flag2, 0x00);
  ASSERT_EQ(hdr->drep1, 0x10);
  ASSERT_EQ(hdr->drep2, 0x00);
  ASSERT_EQ(hdr->drep3, 0x00);
  ASSERT_EQ(hdr->serial_high, 0x00);
  ASSERT_EQ(hdr->serial_low, 0x00);

  const rpc_uuid_t object = {.form{
      .data1 = 0xDEA00000,
      .data2 = 0x6C97,
      .data3 = 0x11D1,
      .data4 = {0x82, 0x71, 0x00, 0x01, 0x03, 0x06, 0x00, 0x2A},
  }};
  ASSERT_EQ(memcmp(&hdr->object_uuid, &object, sizeof(rpc_uuid_t)), 0);
  ASSERT_EQ(hdr->seq_numb, 1126);
  ASSERT_EQ(hdr->operation_numb, 5);
  ASSERT_EQ(hdr->frag_numb, 0);
  ASSERT_EQ(hdr->length_of_body, 84);
}

TEST_F(rpc, rpc_implicit_read) {
  int called_cnt = 0;
  const char read_implicit_req[] =
      "30b8510fe7c4c87f549d22120800450000c004650000ff1130bcc0a80202c0a802b98894c0c000ac724404002000100000000000a0de976c"
      "d111827100010306002a0100a0de976cd111827100a02442df7d80d22eb6551eb21101000800067368280000000001000000660400000500"
      "ffffffff54000000000040800000400000004080000000000000400000000009003c01000000000000000000000000000000000000000000"
      "0000000000010000f82100008000000000000000000000000000000000000000000000000000";
  const char read_implicit_rsp[] =
      "c87f549d221230b8510fe7c40800450000cc04650000ff1130b0c0a802b9c0a80202c0c0889400b8997b04020a00100000000000a0de976c"
      "d111827100010306002a0100a0de976cd111827100a02442df7d80d22eb6551eb21101000800067368280a00000001000000660400000500"
      "ffffffff600000000000000000004c00000040800000000000004c0000008009003c01000000000000000000000000000000000000000000"
      "0000000000010000f8210000000c000000000000000000000000000000000000000000000000001a00080100000100000000";
  auto freq = make_frame(read_implicit_req);
  freq->erase(freq->begin(), freq->begin() + 0x2A);
  auto frsp = make_frame(read_implicit_rsp);
  frsp->erase(frsp->begin(), frsp->begin() + 0x2A);

  rpc_general_req_fn = [&](struct rpc_channel* ch, void* payload, unsigned length) {
    called_cnt++;
    ASSERT_EQ(length, 64);
  };

  ASSERT_EQ(rpc_input(&ctx, freq->data(), freq->size(), 0x0, 0x0, 0x0), 0);
  ASSERT_EQ(called_cnt, 1);

  // generate response
  void* p;
  uint32_t p_len;
  udp_out_fn = [&](void* payload, int length, uint32_t remote_ip, uint16_t remote_port, uint16_t host_port) -> int {
    p = payload;
    p_len = length;
    return 0;
  };

  ctx.channels[0].rsp_pkt_type = RPC_PKT_TYPE_RESP;
  ASSERT_EQ(rpc_output(ctx.channels + 0, frsp->data() + sizeof(struct rpc_hdr), frsp->size() - sizeof(struct rpc_hdr)),
            0);
  ASSERT_EQ(p_len, frsp->size());

  /* FIXME: Boot time not supported yet, correct it manually */
  ctx.channels->output_buf[56] = 10;

  for (int i = 0; i < p_len; i++) {
    uint8_t a = ((uint8_t*)p)[i];
    uint8_t b = frsp->at(i);
    if (a != b) {
      fprintf(stderr, "i: %d, a: %02X, b: %02X\n", i, a, b);
    }
    EXPECT_EQ(a, b);
  }
}
