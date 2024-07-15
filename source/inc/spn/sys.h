#pragma once

#include <lwip/sys.h>

#define SPN_PP_HTONS(x) PP_HTONS(x)
#define SPN_PP_HTONL(x) PP_HTONL(x)
#define SPN_PP_NTOHS(x) PP_NTOHS(x)
#define SPN_PP_NTOHL(x) PP_NTOHL(x)

#define SPN_HTONS(x) __builtin_bswap16(x)
#define SPN_HTONL(x) __builtin_bswap32(x)
#define SPN_NTOHS(x) __builtin_bswap16(x)
#define SPN_NTOHL(x) __builtin_bswap32(x)

#define SPN_ASSERT(msg, cond) LWIP_ASSERT(msg, cond)
#define SPN_DEBUG_MSG(enable, fmt, ...) LWIP_DEBUGF(enable, (fmt, ##__VA_ARGS__))
