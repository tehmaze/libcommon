#ifndef _COMMON_SCAN_H
#define _COMMON_SCAN_H

#include <inttypes.h>
#include <stddef.h>
#include "common.h"
#include "common/scan.h"
#include "common/socket.h"

#if defined(__cplusplus)
extern "C" {
#endif

int          scan_fromhex(unsigned char c);
size_t       scan_ulong(const char *src, unsigned long int *dst);
size_t       scan_xlong(const char *src, unsigned long *dst);
unsigned int scan_ip4(const char *src, ip4_t ip);
unsigned int scan_ip6(const char *src, ip6_t ip);
unsigned int scan_ip6_flat(const char *src, ip6_t ip);

#if defined(__cplusplus)
}
#endif

#endif // _COMMON_SCAN_H
