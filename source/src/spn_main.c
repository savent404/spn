#include <spn.h>
#include <spn/errno.h>
#include <stdlib.h>
#include <string.h>

static spn_instance_t* g_inst = NULL;

int spn_init(spn_instance_t* inst, const uint8_t* mac, const uint8_t* ip)
{
    if (g_inst) {
        return -SPN_EEXIST;
    }

    memset(inst, 0, sizeof(spn_instance_t));
    for (int i = 0; i < SPN_IFACE_MAX_NUM; i++) {
        inst->ifaces[i].port = i;
        inst->ifaces[i].inst = inst;
        memcpy(inst->ifaces[i].mac, mac, 6);
        if (ip) {
            memcpy(inst->ifaces[i].ip, ip, 4);
        }
    }

    g_inst = inst;
    return SPN_OK;
}

int spn_deinit(spn_instance_t* inst)
{
    if (!g_inst || g_inst != inst) {
        return -SPN_EINVAL;
    }
    g_inst = NULL;
    return SPN_OK;
}

spn_instance_t* spn_get_inst()
{
    return g_inst;
}
