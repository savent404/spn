#include <spn/dcp.h>
#include <spn/errno.h>

int spn_dcp_set_req_parse(void* payload, uint16_t len, struct spn_dcp_block_req* reqs, iface_t* iface)
{
    /* Re-use ident.req parser */
    return spn_dcp_ident_req_parse(payload, len, reqs, iface);
}
