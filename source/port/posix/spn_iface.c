#include <spn/config.h>

#include <lwip/netif.h>
#include <lwip/pbuf.h>
#include <netif/ethernet.h>
#include <spn/iface.h>
#include <spn/sys.h>

typedef struct spn_priv_frame {
  struct pbuf* pbuf;
  spn_ftype_t type;
  uint8_t busy;
} spn_priv_frame_t;

static spn_priv_frame_t spn_frames[SPN_CONF_MAXIUM_FRAME_NUM] = {};

static inline spn_priv_frame_t* find_free_frame(void) {
  unsigned i;
  for (i = 0; i < SPN_CONF_MAXIUM_FRAME_NUM; i++) {
    if (!spn_frames[i].busy) {
      spn_frames[i].busy = 1;
      return &spn_frames[i];
    }
  }
  return NULL;
}

spn_frame_t spn_alloc_frame(spn_ftype_t type) {
  spn_priv_frame_t* priv;

  priv = find_free_frame();
  SPN_ASSERT("No free frame", priv != NULL);

  // TODO: RTC need fast path
  priv->pbuf = pbuf_alloc(PBUF_LINK, 1516, PBUF_RAM);
  priv->type = type;

  return priv;
}

void spn_free_frame(spn_frame_t frame) {
  spn_priv_frame_t* priv = (spn_priv_frame_t*)frame;

  SPN_ASSERT("Invalid frame", priv != NULL);

  pbuf_free(priv->pbuf);
  priv->busy = 0;
}

int spn_send_frame(spn_iface_t* iface, spn_frame_t frame, const uint8_t* mac) {
  spn_priv_frame_t* priv = (spn_priv_frame_t*)frame;
  int res;

  SPN_UNUSED_ARG(iface);

  SPN_ASSERT("Invalid frame", priv != NULL);

  // TODO: Reserve time slot for RTC frame
  res = ethernet_output(&iface->netif, priv->pbuf, (const struct eth_addr*)&iface->netif.hwaddr,
                        (const struct eth_addr*)mac, (ETHTYPE_PROFINET));

  return res;
}

void* spn_frame_data(spn_frame_t frame) {
  spn_priv_frame_t* priv = (spn_priv_frame_t*)frame;

  SPN_ASSERT("Invalid frame", priv != NULL);

  return priv->pbuf->payload;
}

uint16_t spn_frame_size(spn_frame_t frame) {
  spn_priv_frame_t* priv = (spn_priv_frame_t*)frame;

  SPN_ASSERT("Invalid frame", priv != NULL);

  return priv->pbuf->tot_len;
}

void spn_frame_set_size(spn_frame_t frame, uint16_t size) {
  spn_priv_frame_t* priv = (spn_priv_frame_t*)frame;

  SPN_ASSERT("Invalid frame", priv != NULL);

  priv->pbuf->tot_len = size;
}

spn_ftype_t spn_frame_type(spn_frame_t frame) {
  spn_priv_frame_t* priv = (spn_priv_frame_t*)frame;

  SPN_ASSERT("Invalid frame", priv != NULL);

  return priv->type;
}

int spn_iface_set_addr(spn_iface_t* iface, uint32_t ip, uint32_t mask, uint32_t gw) {
  ip_addr_t i = {ip}, m = {mask}, g = {gw};
  netif_set_addr(&iface->netif, &i, &m, &g);
  return 0;
}
