#pragma once

#ifndef SPN_EIDIAN
#define SPN_EIDIAN (0) // Little-endian
#endif

#if SPN_EIDIAN == 0
#define SPN_HTONS(x) __builtin_bswap16(x)
#define SPN_HTONL(x) __builtin_bswap32(x)
#define SPN_NTOHS(x) __builtin_bswap16(x)
#define SPN_NTOHL(x) __builtin_bswap32(x)
#else
#define SPN_HTONS(x) (x)
#define SPN_HTONL(x) (x)
#define SPN_NTOHS(x) (x)
#define SPN_NTOHL(x) (x)
#endif

#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)