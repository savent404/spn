#include "lwip/opt.h"
#include "lwipopts.h"

#include "arch/sys_arch.h"
#include "lwip/api.h"
#include "lwip/etharp.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/sys.h"
#include "lwip/tcp.h"
#include "lwip/tcpip.h"
#include "lwip/timeouts.h"
#include "lwip/udp.h"
#include "netif/ethernet.h"
#include "lwip/icmp.h"
#include "lwip/igmp.h"

#include "posix_port_netif.h"

static void tcpip_init_cb(void* arg);

int main(void)
{
    err_t err;
    sys_sem_t stack_init_sem;

    err = sys_sem_new(&stack_init_sem, 0);
    LWIP_ASSERT("Failed to create semaphore", err == ERR_OK);

    tcpip_init(tcpip_init_cb, &stack_init_sem);

    sys_sem_wait(&stack_init_sem);
    sys_sem_free(&stack_init_sem);

    while (!lwip_unix_keypressed()) {
        /* default_netif_poll(); */
    }

    /* default_netif_shutdown(); */
    return 0;
}

void tcpip_init_cb(void* arg)
{
    sys_sem_t* sem = (sys_sem_t*)arg;
    sys_sem_signal(sem);

    default_netif_init();
    etharp_init();
    udp_init();
}
