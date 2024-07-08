#include <chrono>
#include <lwip/opt.h>

#include "dummy.hpp"
#include <gtest/gtest.h>
#include <lwip/etharp.h>
#include <lwip/tcpip.h>
#include <thread>

extern "C" {

static err_t low_level_output(struct netif* netif, struct pbuf* p)
{
    SpnInstance* inst = (SpnInstance*)netif->state;

    return inst->cb_low_level_output(netif, p);
}

static err_t low_level_input(struct netif* netif, struct pbuf* p)
{
    SpnInstance* inst = (SpnInstance*)netif->state;

    return inst->cb_low_level_input(netif, p);
}

static err_t low_level_init(struct netif* netif)
{
    SpnInstance* inst = (SpnInstance*)netif->state;

    netif->mtu = 1500;
    netif->hwaddr_len = 6;
    netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
    netif->hwaddr[0] = 0x00;
    netif->hwaddr[1] = 0x0c;
    netif->hwaddr[2] = 0x29;
    netif->hwaddr[3] = 0xcd;
    netif->hwaddr[4] = 0xDD;
    netif->hwaddr[5] = rand() & 0xFF;

    netif->output = etharp_output;
    netif->linkoutput = low_level_output;
    if (inst->cb_low_level_init)
        return inst->cb_low_level_init(netif);
    return ERR_OK;
}

static err_t low_level_poll(struct netif* netif)
{
    SpnInstance* inst = (SpnInstance*)netif->state;

    return inst->cb_low_level_poll(netif);
}
}

bool LwipInstance::initialized = false;
std::promise<void> LwipInstance::promise;
std::future<void> LwipInstance::future;

LwipInstance::LwipInstance()
{
    if (!initialized) {
        tcpip_init(tcpip_init_cb, NULL);
        future = promise.get_future();
        future.wait();
        initialized = true;
    }
}

void LwipInstance::tcpip_init_cb(void* arg)
{
    promise.set_value();
}

SpnInstance::~SpnInstance()
{
    spn_deinit(&ctx);
}

void SpnInstance::SetUp()
{
    LOCK_TCPIP_CORE();
    spn_init(&ctx, &cfg, &ifaces[0], NULL);
    for (auto& iface : ifaces) {
        netif_add(&iface, NULL, NULL, NULL, (void*)this, low_level_init, tcpip_input);
        netif_set_link_up(&iface);
        netif_set_up(&iface);
    }
    UNLOCK_TCPIP_CORE();
}

void SpnInstance::TearDown()
{
    LOCK_TCPIP_CORE();
    for (auto& iface : ifaces) {
        netif_remove(&iface);
    }
    UNLOCK_TCPIP_CORE();
}

bool SpnInstance::step()
{
    err_t res;

    assert(cb_low_level_poll);

    res = cb_low_level_poll(&ifaces[0]);
    std::this_thread::yield();
    if (res != ERR_OK) {
        return false;
    }

    if (cfg.dual_port) {
        res = cb_low_level_poll(&ifaces[1]);
        std::this_thread::yield();
        if (res != ERR_OK) {
            return false;
        }
    }
    return true;
}
