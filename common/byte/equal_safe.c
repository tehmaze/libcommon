#include "common/byte.h"

PRIVATE bool byte_equal_safe(const void *a, const void *b, size_t len)
{
    size_t i;
    uint8_t r = 0;
    uint8_t *x = (uint8_t *)a;
    uint8_t *y = (uint8_t *)b;
    for (i = 0; i < len; i++) {
        r |= (x[i] ^ y[i]);
    }
    return r == 0;
}
