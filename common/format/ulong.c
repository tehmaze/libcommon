#include "common/format.h"

PRIVATE size_t format_ulong(char *dst, unsigned long i)
{
    register unsigned long len, tmp, len2;
    /* first count the number of bytes needed */
    for (len = 1, tmp = i; tmp > 9; ++len) {
        tmp /= 10;
    }
    if (dst) {
        for (tmp = i, dst += len, len2 = len + 1; --len2; tmp /= 10) {
            *--dst = (tmp%10)+'0';
        }
    }
    return len;
}
