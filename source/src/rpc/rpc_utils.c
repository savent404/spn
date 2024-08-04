#include <spn/rpc.h>
#include <spn/sys.h>

static void rpc_uuid_swap(rpc_uuid_t* uuid) {
  uuid->form.data1 = SPN_NTOHL(uuid->form.data1);
  uuid->form.data2 = SPN_NTOHS(uuid->form.data2);
  uuid->form.data3 = SPN_NTOHS(uuid->form.data3);
}

int rpc_is_big_endian(struct rpc_hdr* hdr) {
  const uint8_t encoding = (hdr->drep1 & 0x70) & 0x07; /* bit[4:7] 0-BE, 1-LE */
  return !encoding;
}

void rpc_hton(struct rpc_hdr* hdr) {
  const uint8_t encoding = (hdr->drep1 & 0x70) & 0x07; /* bit[4:7] 0-BE, 1-LE */
  SPN_ASSERT("invalid drep BE/LE flag", encoding < 2);

  if (encoding) {
    /* Hdr is LE, no need to do anything */
    return;
  }

  rpc_uuid_swap(&hdr->object_uuid);
  rpc_uuid_swap(&hdr->interface_uuid);
  rpc_uuid_swap(&hdr->activity_uuid);
  hdr->boot_time = SPN_HTONL(hdr->boot_time);
  hdr->interface_version_major = SPN_HTONS(hdr->interface_version_major);
  hdr->interface_version_minor = SPN_HTONS(hdr->interface_version_minor);
  hdr->seq_numb = SPN_HTONL(hdr->seq_numb);
  hdr->operation_numb = SPN_HTONS(hdr->operation_numb);
  hdr->interface_hint = SPN_HTONS(hdr->interface_hint);
  hdr->activity_hint = SPN_HTONS(hdr->activity_hint);
  hdr->length_of_body = SPN_HTONS(hdr->length_of_body);
  hdr->frag_numb = SPN_HTONS(hdr->frag_numb);
}

void rpc_ntoh(struct rpc_hdr* hdr) {
  rpc_hton(hdr);
}

void rpc_ndr_ntoh(void* ndr_data, rpc_pkt_type_t type) {
  switch (type) {
    case RPC_PKT_TYPE_REQ: {
      struct rpc_ndr_data_req* hdr = (struct rpc_ndr_data_req*)ndr_data;
      hdr->args_maxium = SPN_NTOHL(hdr->args_maxium);
      hdr->args_length = SPN_NTOHL(hdr->args_length);
      hdr->maxium_count = SPN_NTOHL(hdr->maxium_count);
      hdr->offset = SPN_NTOHL(hdr->offset);
      hdr->actual_count = SPN_NTOHL(hdr->actual_count);
      break;
    }
    default:
      SPN_ASSERT("Unknow pkt type need to be swap", 0);
      break;
  }
}

void rpc_ndr_hton(void* ndr_data, rpc_pkt_type_t type) {
  rpc_ndr_ntoh(ndr_data, type);
}
