#include <lwip/opt.h>

#include <lwip/debug.h>
#include <lwip/opt.h>
#include <lwip/pbuf.h>
#include <spn/config.h>
#include <spn/errno.h>
#include <spn/pdu.h>
#include <spn/spn.h>
#include <string.h>

#include <lwip/prot/ethernet.h>

__attribute__((weak)) void _spn_input_indication(int result)
{
    LWIP_UNUSED_ARG(result);
}

int spn_init(struct spn_ctx* ctx, const struct spn_cfg* cfg, iface_t* if1, iface_t* if2)
{
    if (!ctx || !cfg || !if1) {
        return SPN_EINVAL;
    }

    ctx->cfg = cfg;

    if (cfg->dual_port) {
        if (!if2) {
            return SPN_EINVAL;
        }
        ctx->iface_port1 = if1;
        ctx->iface_port2 = if2;
    } else {
        ctx->iface_port1 = if1;
        ctx->iface_port2 = NULL;
    }

    return SPN_OK;
}

void spn_deinit(struct spn_ctx* ctx)
{
    if (ctx) {
        ctx->iface_port1 = NULL;
        ctx->iface_port2 = NULL;
    }
}

int spn_input_hook(void* frame, void* iface)
{
    struct pbuf* p = (struct pbuf*)frame;
    struct eth_hdr* hdr = (struct eth_hdr*)p->payload;
    iface_t* i = (iface_t*)iface;
    int res;

    if (p && i) {
        if (hdr->type == PP_HTONS(ETHTYPE_PROFINET)) {
            LWIP_DEBUGF(SPN_DEBUG | LWIP_DBG_TRACE, ("SPN: pn frame...\n"));
            res = spn_pdu_input((char*)p->payload + SIZEOF_ETH_HDR, p->len - SIZEOF_ETH_HDR, hdr, i);
        } else {
            LWIP_DEBUGF(SPN_DEBUG | LWIP_DBG_TRACE, ("SPN: unknow ethernet type: %X\n", lwip_htons(hdr->type)));
            res = -SPN_EAGAIN;
        }
    } else {
        res = -SPN_EINVAL;
    }

    if (res == SPN_OK) {
        pbuf_free(p);
    }

    /* Test purpose hook */
    _spn_input_indication(res);

    return res;
}
