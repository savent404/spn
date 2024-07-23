#include "lwip/api.h"
#include "lwip/netif.h"
#include <getopt.h>
#include <spn/spn.h>
#include <spn/sys.h>
#include <string.h>
#include "posix_port_netif.h"

// lwip apps
#include "lwip/etharp.h"
#include "lwip/apps/snmp.h"
#include "lwip/tcpip.h"
#include "lwip/udp.h"


static void tcpip_init_cb(void* arg);
static void parse_args(int argc, char** argv);

static struct spn_ctx ctx = {0};
static const char *port1_name = "", *port2_name = "";
static uint32_t ip = 0xF91FA8C0;
static struct netif ifaces[2];

int main(int argc, char** argv) {
  err_t err;
  sys_sem_t stack_init_sem;
  struct spn_cfg cfg = {0};

  parse_args(argc, argv);

  spn_init(&ctx, &cfg);

  err = sys_sem_new(&stack_init_sem, 0);
  LWIP_ASSERT("Failed to create semaphore", err == ERR_OK);

  tcpip_init(tcpip_init_cb, &stack_init_sem);

  sys_sem_wait(&stack_init_sem);
  sys_sem_free(&stack_init_sem);

  while (1) {
    /* default_netif_poll(); */
    sys_check_timeouts();
    sys_msleep(1);
  }

  /* default_netif_shutdown(); */
  return 0;
}

void tcpip_init_cb(void* arg) {
  sys_sem_t* sem = (sys_sem_t*)arg;

  sys_sem_signal(sem);

  etharp_init();
  udp_init();
  // snmp_init();

  LWIP_DEBUGF(0x80, ("TCP/IP initialized.\n"));
}

int spn_port_init(struct spn_ctx* ctx, struct spn_iface* iface, uint16_t interface, uint16_t port) {
  const char* name;
  SPN_ASSERT("Not supported interface or port", interface == 0 && port < 2);

  name = port == 0 ? port1_name : port2_name;
  if (!name || !strlen(name)) {
    return -SPN_ENOENT;
  }
  dft_port_init(&iface->netif, name, ip);
  return SPN_OK;
}

void parse_args(int argc, char** argv) {
  int c;

  while (1) {
    static struct option long_options[] = {{"port1", required_argument, 0, '1'},
                                           {"port2", optional_argument, 0, '2'},
                                           {"ip", required_argument, 0, 'i'},
                                           {"help", no_argument, 0, 'h'},
                                           {0, 0, 0, 0}};

    int option_index = 0;
    c = getopt_long(argc, argv, "1:2:i:h", long_options, &option_index);

    if (c == -1)
      break;

    switch (c) {
      case '1':
        port1_name = optarg;
        break;
      case '2':
        port2_name = optarg;
        break;
      case 'i': {
        int ips[4];
        int i;
        sscanf(optarg, "%d.%d.%d.%d", &ips[0], &ips[1], &ips[2], &ips[3]);
        for (i = 0; i < 4; i++) {
          ips[i] &= 0xFF;
        }
        ip = (ips[0]) | (ips[1] << 8) | (ips[2] << 16) | (ips[3] << 24);
        break;
      }
      case 'h':
        // print usage
        printf("Usage: %s --port1 <port1_name> --port2 <port2_name>\n", argv[0]);
        exit(0);
      default:
        printf("Unknown option\n");
        exit(1);
    }
  }
}
