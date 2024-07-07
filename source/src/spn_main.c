#include <spn/spn.h>

int spn_init(struct spn_ctx *ctx, const struct spn_cfg *cfg)
{
    if (!ctx || !cfg) {
        return SPN_EINVAL;
    }
    return SPN_OK;
}

int spn_input_hook(void *frame, void *iface)
{
    frame = frame;
    iface = iface;
    return -1;
}
