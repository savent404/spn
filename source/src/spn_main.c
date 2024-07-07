#include <spn/spn.h>

int spn_init(struct spn_ctx* ctx, const struct spn_cfg* cfg, iface_t *if1, iface_t *if2)
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

int spn_input_hook(void* frame, void* iface)
{
    struct pbuf* p = (struct pbuf*) frame;
    iface = iface;
    p = p;
    return 1;
}
