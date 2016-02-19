#include "common/byte.h"

PRIVATE int8_t byte_cmp(const void *a, const void *b, size_t len)
{
    size_t i;
    uint8_t *x = (uint8_t *)a, *y = (uint8_t *)b;
    for (i = 0; i < len; i++) {
        if (x[i] > y[i])
            return 1;
        else if (x[i] < y[i])
            return -1;
    }
    return 0;
}
