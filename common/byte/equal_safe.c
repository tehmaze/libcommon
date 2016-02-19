#include "common/byte.h"

PRIVATE bool byte_equal_safe(const void *a, const void *b, size_t len)
{
    size_t i;
    uint8_t r = 0;
    for (i = 0; i < len; i++) {
        r |= ((uint8_t)(a + i) ^ (uint8_t)(b + i));
    }
    return r == 0;
}
