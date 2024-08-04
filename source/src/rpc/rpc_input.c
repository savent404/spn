#include <spn/rpc.h>
#include <spn/sys.h>

#if 0
#include <endian.h>
#if __BYTE_ORDER == __BIG_ENDIAN
#error "big endian is not supported"
#endif
#endif

// clang-format off
const rpc_uuid_t device_interface = {
    .raw = {
        0xDE, 0xA0, 0x00, 0x01,
        0x6C, 0x97,
        0x11, 0xD1,
        0x82, 0x71, 0x00, 0xA0, 0x24, 0x42, 0xDF, 0x7D
    }
};

const rpc_uuid_t controller_interafce = {
    .raw = {
        0xDE, 0xA0, 0x00, 0x02,
        0x6C, 0x97,
        0x11, 0xD1,
        0x82, 0x71, 0x00, 0xA0, 0x24, 0x42, 0xDF, 0x7D
    }
};

const rpc_uuid_t supervisor_interface = {
    .raw = {
        0xDE, 0xA0, 0x00, 0x03,
        0x6C, 0x97,
        0x11, 0xD1,
        0x82, 0x71, 0x00, 0xA0, 0x24, 0x42, 0xDF, 0x7D
    }
};

const rpc_uuid_t parameter_server_interface = {
    .raw = {
        0xDE, 0xA0, 0x00, 0x04,
        0x6C, 0x97,
        0x11, 0xD1,
        0x82, 0x71, 0x00, 0xA0, 0x24, 0x42, 0xDF, 0x7D
    }
};

const rpc_uuid_t cim_interface = {
    .raw = {
        0xDE, 0xA0, 0x00, 0x05,
        0x6C, 0x97,
        0x11, 0xD1,
        0x82, 0x71, 0x00, 0xA0, 0x24, 0x42, 0xDF, 0x7D
    }
};

const rpc_uuid_t epmap_interface = {
    .raw = {
        0xE1, 0xAF, 0x83, 0x08,
        0x5D, 0x1F,
        0x11, 0xC9,
        0x91, 0xA4, 0x08, 0x00, 0x2B, 0x14, 0xA0, 0xFA
    }
};

const rpc_uuid_t epmap_object = {
    .raw = {
        0x00, 0x00, 0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }
};
// clang-format on

static void rpc_uuid_swap(rpc_uuid_t* uuid) {
  SPN_UNUSED_ARG(uuid);
}

static void rpc_hdr_hton(struct rpc_hdr* hdr) {
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
  hdr->operation_numb = SPN_HTONL(hdr->operation_numb);
  hdr->interface_hint = SPN_HTONS(hdr->interface_hint);
  hdr->activity_hint = SPN_HTONS(hdr->activity_hint);
  hdr->length_of_body = SPN_HTONS(hdr->length_of_body);
  hdr->frag_numb = SPN_HTONS(hdr->frag_numb);
}

#if 0
static void rpc_hdr_ntoh(struct rpc_hdr* hdr) {
  const uint8_t encoding = (hdr->drep1 & 0x70) & 0x07; /* bit[4:7] 0-BE, 1-LE */
  SPN_ASSERT("invalid drep BE/LE flag", encoding < 2);

  if (encoding) {
    /* Hdr is LE, no need to do anything */
    return;
  }

  rpc_uuid_swap(&hdr->object_uuid);
  rpc_uuid_swap(&hdr->interface_uuid);
  rpc_uuid_swap(&hdr->activity_uuid);
  hdr->boot_time = SPN_NTOHL(hdr->boot_time);
  hdr->interface_version_major = SPN_NTOHS(hdr->interface_version_major);
  hdr->interface_version_minor = SPN_NTOHS(hdr->interface_version_minor);
  hdr->seq_numb = SPN_NTOHL(hdr->seq_numb);
  hdr->operation_numb = SPN_NTOHL(hdr->operation_numb);
  hdr->interface_hint = SPN_NTOHS(hdr->interface_hint);
  hdr->activity_hint = SPN_NTOHS(hdr->activity_hint);
  hdr->length_of_body = SPN_NTOHS(hdr->length_of_body);
  hdr->frag_numb = SPN_NTOHS(hdr->frag_numb);
}
#endif

int rpc_input(struct rpc_ctx* ctx, void* payload, int length) {
  struct rpc_hdr* hdr;
  SPN_UNUSED_ARG(ctx);
  SPN_UNUSED_ARG(length);

  SPN_ASSERT("invalid ctx", ctx);
  SPN_ASSERT("invalid payload", payload && length);

  hdr = (struct rpc_hdr*)payload;

  rpc_hdr_hton(hdr);

  /** TODO: Remove this */
  hdr->boot_time = 0;

  return 0;
}
