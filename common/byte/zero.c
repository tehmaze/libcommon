#include "common/byte.h"

PRIVATE void byte_zero(void *buf, size_t len)
{
    register uint8_t *ptr = buf;
    size_t i;
    for (i = 0; i < len; i++) {
        ptr[i] = 0;
    }
}
