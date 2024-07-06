#include "lwip/opt.h"

#include "lwip/etharp.h"
#include "lwip/ip4_addr.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip/snmp.h"
#include "lwip/sys.h"
#include "lwip/tcpip.h"
#include <arpa/inet.h>
#include <assert.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct raw_posix_iface {
    int sockfd;
    char ifname[8];
    struct sockaddr_ll sockaddr;
};

static err_t raw_socket_low_level_init(struct netif* netif);
static err_t raw_socket_poll(struct netif* netif);
static err_t raw_low_level_output(struct netif* netif, struct pbuf* p);
static struct pbuf* raw_low_level_input(struct netif* netif);
static void* raw_socket_background_thread(void* arg);

static pthread_t background_thread;

void netif_init(void)
{
    ip4_addr_t ipaddr, netmask, gw;
    static struct netif netif[2];
    static struct raw_posix_iface iface[2];

    strcpy(iface[0].ifname, "ens37");
    strcpy(iface[1].ifname, "ens38");

    ip4_addr_set_zero(&gw);
    ip4_addr_set_zero(&ipaddr);
    ip4_addr_set_zero(&netmask);

    IP4_ADDR((&ipaddr), 192, 168, 31, 249);
    IP4_ADDR((&netmask), 255, 255, 255, 0);
    IP4_ADDR((&gw), 192, 168, 31, 1);
    printf("Starting lwIP, local interface IP is %s\n", ip4addr_ntoa(&ipaddr));
    /* TODO: add second port */
    netif_add(&netif[0], &ipaddr, &netmask, &gw, &iface[0], raw_socket_low_level_init, tcpip_input);
    netif_set_default(&netif[0]);

    pthread_create(&background_thread, NULL, raw_socket_background_thread, &netif[0]);
}

err_t raw_socket_low_level_init(struct netif* netif)
{
    struct raw_posix_iface* iface = (struct raw_posix_iface*)netif->state;
    struct ifreq ifr;
    int sockfd;

    /* Create a raw socket for L2 frames */
    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sockfd < 0) {
        perror("socket");
        return ERR_IF;
    }

    memset(&ifr, 0, sizeof(ifr));
    memset(&iface->sockaddr, 0, sizeof(iface->sockaddr));
    iface->sockaddr.sll_ifindex = if_nametoindex(iface->ifname);
    strcpy(ifr.ifr_name, iface->ifname);
    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)) < 0) {
        perror("setsockopt");
        return ERR_IF;
    }

    /* Setup netif's driver functions */
    netif->output = etharp_output;
    netif->linkoutput = raw_low_level_output;

    /* Setup netif's default value */
    netif->mtu = 1500;
    netif->hwaddr_len = 6;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
    netif->hwaddr[0] = 0x00;
    netif->hwaddr[1] = 0x0c;
    netif->hwaddr[2] = 0x29;
    netif->hwaddr[3] = 0x55;
    netif->hwaddr[4] = 0x44;
    netif->hwaddr[5] = 0x33;
    iface->sockfd = sockfd;

    netif_set_link_up(netif);

    return ERR_OK;
}

struct pbuf* raw_low_level_input(struct netif* netif)
{
    struct raw_posix_iface* iface = (struct raw_posix_iface*)netif->state;
    ssize_t len;
    struct pbuf* q;
    static char buf[1518];

    len = recvfrom(iface->sockfd, buf, sizeof(buf), 0, NULL, NULL);
    if (len < 0) {
        perror("recvfrom");
        return NULL;
    }
    MIB2_STATS_NETIF_ADD(netif, ifinoctets, len);

    q = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
    if (q != NULL) {
        pbuf_take(q, buf, len);
    } else {
        MIB2_STATS_NETIF_INC(netif, ifindiscards);
    }

    return q;
}

err_t raw_socket_poll(struct netif* netif)
{
    struct pbuf* p;

    p = raw_low_level_input(netif);
    if (p == NULL) {
        return ERR_IF;
    }

    if (netif->input(p, netif) != ERR_OK) {
        pbuf_free(p);
    }

    return ERR_OK;
}

err_t raw_low_level_output(struct netif* netif, struct pbuf* p)
{
    struct raw_posix_iface* iface = (struct raw_posix_iface*)netif->state;
    ssize_t len;
    static char buf[1518];
    int i;

    pbuf_copy_partial(p, buf, p->tot_len, 0);

    len = sendto(iface->sockfd, buf, p->tot_len, 0, (struct sockaddr*)&iface->sockaddr, sizeof(iface->sockaddr));
    if (len < p->tot_len) {
        MIB2_STATS_NETIF_INC(netif, ifoutdiscards);
        perror("sendto");
        /* Dump frame like xxd format */
        for (i = 0; i < p->tot_len; i++) {
            printf("%02x ", (unsigned char)buf[i]);
            if (i % 16 == 15)
                printf("\n");
        }
        return ERR_IF;
    }
    MIB2_STATS_NETIF_ADD(netif, ifoutoctets, len);
    return ERR_OK;
}

void* raw_socket_background_thread(void* arg)
{
    struct netif* netif = (struct netif*)arg;

    sys_msleep(1e3);

    while (1) {
        raw_socket_poll(netif);
    }

    return NULL;
}
