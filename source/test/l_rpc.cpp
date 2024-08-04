#include <gtest/gtest.h>
#include <spn/errno.h>
#include <spn/rpc.h>
#include <string.h>
#include <unistd.h>
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
}  // namespace

TEST(rpc, ntoh) {
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
    rpc_ntoh(hdr);
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