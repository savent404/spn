#include "lwip/opt.h"
#include "lwipopts.h"

#include "arch/sys_arch.h"
#include "lwip/api.h"
#include "lwip/etharp.h"
#include "lwip/icmp.h"
#include "lwip/igmp.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/tcp.h"
#include "lwip/tcpip.h"
#include "lwip/timeouts.h"
#include "lwip/udp.h"
#include "netif/ethernet.h"

#include "posix_port_netif.h"
#include <spn/spn.h>

static void tcpip_init_cb(void* arg);

static struct spn_ctx ctx = { 0 };

static struct netif ifaces[2];

int main(void)
{
    err_t err;
    sys_sem_t stack_init_sem;
    struct spn_cfg cfg = { 0 };

    spn_init(&ctx, &cfg, &ifaces[0], NULL);

    err = sys_sem_new(&stack_init_sem, 0);
    LWIP_ASSERT("Failed to create semaphore", err == ERR_OK);

    tcpip_init(tcpip_init_cb, &stack_init_sem);

    sys_sem_wait(&stack_init_sem);
    sys_sem_free(&stack_init_sem);

    while (1) {
        /* default_netif_poll(); */
        sys_msleep(1000);
    }

    /* default_netif_shutdown(); */
    return 0;
}

void tcpip_init_cb(void* arg)
{
    sys_sem_t* sem = (sys_sem_t*)arg;
    sys_sem_signal(sem);

    default_netif_init((struct netif*)ifaces);

    etharp_init();
    udp_init();
}
