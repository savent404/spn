#include <spn/config.h>

#include <lwip/ip_addr.h>
#include <lwip/netif.h>
#include <lwip/netif.h>
#include <lwip/pbuf.h>
#include <netif/ethernet.h>
#include <spn/iface.h>
#include <spn/sys.h>
#include <cstring>
#include <list>
#include <memory>
#include "t_iface.hpp"

int spn_iface_set_addr(spn_iface_t* iface, uint32_t ip, uint32_t mask, uint32_t gw) {
  ip4_addr_t i, m, g;
  i.addr = ip;
  m.addr = mask;
  g.addr = gw;
  netif_set_addr(&iface->netif, &i, &m, &g);
  SPN_DEBUG_MSG(SPN_DCP_DEBUG, "set_ind: set IP address: %s\n", ipaddr_ntoa(&iface->netif.ip_addr));
  SPN_DEBUG_MSG(SPN_DCP_DEBUG, "set_ind: set netmask: %s\n", ipaddr_ntoa(&iface->netif.netmask));
  SPN_DEBUG_MSG(SPN_DCP_DEBUG, "set_ind: set gateway: %s\n", ipaddr_ntoa(&iface->netif.gw));

  return 0;
}

spn_frame_t spn_alloc_frame(spn_ftype_t type) {
  auto frame = new iface_instace::spn_priv_frame;
  frame->type = type;
  return frame;
}

void spn_free_frame(spn_frame_t frame) {
  delete (iface_instace::spn_priv_frame*)frame;
}

int spn_send_frame(spn_iface_t* iface, spn_frame_t frame, const uint8_t* mac) {
  auto instance = iface_instace::get_instance();
  auto new_priv = new iface_instace::spn_priv_frame;
  auto priv = (iface_instace::spn_priv_frame*)frame;

  memcpy(new_priv->buff, priv->buff, priv->len);
  new_priv->len = priv->len;
  new_priv->iface = iface;
  memcpy(new_priv->dst, mac, 6);
  instance->fifo.push_back(new_priv);
  return 0;
}

void* spn_frame_data(spn_frame_t frame) {
  auto priv = (iface_instace::spn_priv_frame*)frame;
  return priv->buff;
}

uint16_t spn_frame_size(spn_frame_t frame) {
  auto priv = (iface_instace::spn_priv_frame*)frame;
  return priv->len;
}

void spn_frame_set_size(spn_frame_t frame, uint16_t size) {
  auto priv = (iface_instace::spn_priv_frame*)frame;
  priv->len = size;
}

spn_ftype_t spn_frame_type(spn_frame_t frame) {
  auto priv = (iface_instace::spn_priv_frame*)frame;
  return priv->type;
}
