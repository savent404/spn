#include <spn/rpc.h>
#include <spn/sys.h>

// clang-format off
const rpc_uuid_t object_uuid = {
  .form = {
    .data1 = 0xDEA00000,
    .data2 = 0x6C97,
    .data3 = 0x11D1,
    /* FIXME: 0x82, 0x71, <instance id>, <vendor id>, <device id> */
    .data4 = {0x82, 0x71, 0x00, 0x64, 0x01, 0x19, 0x00, 0x2A}
  }
};

const rpc_uuid_t device_interface = {
    .form = {
      .data1 = 0xDEA00001,
      .data2 = 0x6C97,
      .data3 = 0x11D1,
      /* FIXME: 0x82,0x71,<instance-id>,<vendor id>,<device id>*/
      .data4 = {0x82, 0x71, 0x00, 0xA0, 0x24, 0x42, 0xDF, 0x7D}
    }
};

const rpc_uuid_t controller_interafce = {
    .form = {
        .data1 = 0xDEA00002,
        .data2 = 0x6C97,
        .data3 = 0x11D1,
        .data4 = {0x82, 0x71, 0x00, 0xA0, 0x24, 0x42, 0xDF, 0x7D}
    }
};

const rpc_uuid_t supervisor_interface = {
    .form = {
        .data1 = 0xDEA00003,
        .data2 = 0x6C97,
        .data3 = 0x11D1,
        .data4 = {0x82, 0x71, 0x00, 0xA0, 0x24, 0x42, 0xDF, 0x7D}
    }
};

const rpc_uuid_t parameter_server_interface = {
    .form = {
        .data1 = 0xDEA00004,
        .data2 = 0x6C97,
        .data3 = 0x11D1,
        .data4 = {0x82, 0x71, 0x00, 0xA0, 0x24, 0x42, 0xDF, 0x7D}
    }
};

const rpc_uuid_t cim_interface = {
    .form = {
        .data1 = 0xDEA00005,
        .data2 = 0x6C97,
        .data3 = 0x11D1,
        .data4 = {0x82, 0x71, 0x00, 0xA0, 0x24, 0x42, 0xDF, 0x7D}
    }
};

const rpc_uuid_t epmap_interface = {
    .form = {
        .data1 = 0x8A885D06,
        .data2 = 0x1CEB,
        .data3 = 0x11C9,
        .data4 = {0x9F, 0xE8, 0x08, 0x00, 0x2B, 0x10, 0x48, 0x60}
    }
};

const rpc_uuid_t epmap_object = {
    .form = {
        .data1 = 0x8A885D04,
        .data2 = 0x1CEB,
        .data3 = 0x11C9,
        .data4 = {0x9F, 0xE8, 0x08, 0x00, 0x2B, 0x10, 0x48, 0x60}
    }
};

const rpc_uuid_t* rpc_uuids[] = {
    &device_interface,
    &controller_interafce,
    &supervisor_interface,
    &parameter_server_interface,
    &cim_interface,
    &epmap_interface,
    &epmap_object
};
// clang-format on

static void rpc_uuid_swap(rpc_uuid_t* uuid) {
  uuid->form.data1 = SPN_NTOHL(uuid->form.data1);
  uuid->form.data2 = SPN_NTOHS(uuid->form.data2);
  uuid->form.data3 = SPN_NTOHS(uuid->form.data3);
}

int rpc_is_big_endian(struct rpc_hdr* hdr) {
  const uint8_t encoding = hdr->drep1 >> 4; /* bit[4:7] 0-BE, 1-LE */
  return !encoding;
}

void rpc_hdr_hton(struct rpc_hdr* hdr) {
  const uint8_t encoding = hdr->drep1 >> 4; /* bit[4:7] 0-BE, 1-LE */
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

void rpc_hdr_ntoh(struct rpc_hdr* hdr) {
  rpc_hdr_hton(hdr);
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

const rpc_uuid_t* rpc_get_consistent_uuid(enum rpc_interface_type type) {
  return rpc_uuids[type];
}

const rpc_uuid_t* rpc_get_object_uuid(void) {
  return &object_uuid;
}