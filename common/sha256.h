/**
 * @file
 * @author Igor Pavlov
 * @date 2010-06-11
 */
#ifndef _SHA256_H
#define _SHA256_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <inttypes.h>
#include <stdlib.h>

#define SHA256_DIGEST_LENGTH 32

typedef struct {
    uint32_t state[8];
    uint64_t count;
    uint8_t  buffer[64];
} sha256_t;

void sha256_init(sha256_t *p);
void sha256_update(sha256_t *p, const uint8_t *data, size_t size);
void sha256_final(sha256_t *p, uint8_t *digest);

#if defined(__cplusplus)
}
#endif

#endif // _SHA256_H
