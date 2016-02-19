#include "common/format.h"
#include "common/config.h"

PRIVATE static inline char tohex(char c) {
    return c >= 10 ? c - 10 + 'a' : c + '0';
}

PRIVATE size_t format_xlong(char *dst, unsigned long i) {
    register unsigned long len, tmp;
    /* first count the number of bytes needed */
    for (len = 1, tmp = i; tmp > 15; ++len) {
        tmp >>= 4;
    }
    if (dst) {
        for (tmp = i, dst += len; ; ) {
            *--dst = tohex(tmp & 15);
            if (!(tmp >>= 4)) break;
        }
    }
    return len;
}
