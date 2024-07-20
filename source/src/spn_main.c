#include <spn/spn.h>
#include <spn/sys.h>

int spn_init(struct spn_ctx* ctx, const struct spn_cfg* cfg, iface_t* iface1, iface_t* iface2)
{
    SPN_UNUSED_ARG(ctx);
    SPN_UNUSED_ARG(cfg);
    SPN_UNUSED_ARG(iface1);
    SPN_UNUSED_ARG(iface2);

    return 0;
}

void spn_deinit(struct spn_ctx* ctx)
{
    SPN_UNUSED_ARG(ctx);
}

int spn_input_hook(void* frame, void* iface)
{
    SPN_UNUSED_ARG(frame);
    SPN_UNUSED_ARG(iface);
    _spn_input_indication(0);
    return 0;
}

__attribute__((weak))
void _spn_input_indication(int result)
{
    SPN_UNUSED_ARG(result);
}
