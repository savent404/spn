#pragma once

#include <spn/config.h>

#include <lwip/timeouts.h>

typedef void (*spn_timeout_handler)(void* arg);

#if SPN_TEST

#define SPN_TIMEOUT(msecs, handler, arg)
#define SPN_UNTIMEOUT(handler, arg)

#else

#define SPN_TIMEOUT(msecs, handler, arg) sys_timeout(msecs, handler, arg)
#define SPN_UNTIMEOUT(handler, arg) sys_untimeout(handler, arg)

#endif
