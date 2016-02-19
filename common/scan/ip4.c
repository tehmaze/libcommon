#include "common/scan.h"

PRIVATE unsigned int scan_ip4(const char *src, ip4_t ip)
{
    unsigned int len;
    unsigned long u;
    int i;

    len = 0;
    for (i = 0; i < 4; ++i) {
        register unsigned int j;
        len += (j = scan_ulong(src, &u)) + 1;
        if (!j) {
            return 0;
        }
        ip[i] = u;
        src += j;
        if (i < 3 && *src != '.') {
            return 0;
        }
        src++;
    }
    return len - 1;
}
