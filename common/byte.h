#ifndef _COMMON_BYTE_H
#define _COMMON_BYTE_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include "common.h"

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(MAX)
#define MAX(a,b) ((a > b) ? (a) : (b))
#endif
#if !defined(MIN)
#define MIN(a,b) ((a < b) ? (a) : (b))
#endif

#if defined(__restrict__)
#define ___restrict __restrict__
#elif defined(__restrict)
#define ___restrict __restrict
#else
#define ___restrict
#endif

extern const uint8_t bhex[16];
extern const uint8_t bheX[16];
extern const char    chex[16];
extern const char    cheX[16];

int8_t byte_cmp(const void *___restrict a, const void *___restrict b, size_t len);
void   byte_copy(void *___restrict dst, const void *___restrict src, size_t len);
bool   byte_equal_safe(const void *a, const void *b, size_t len);
void   byte_zero(void *buf, size_t len);

/* Do not use to compare passwords, see byte_equal_safe */
#define byte_equal(a, b, s) (bool)(!byte_cmp((a), (b), (s)))

#if defined(__cplusplus)
}
#endif

#endif // _COMMON_BYTE_H
