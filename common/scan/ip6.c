#include "common/byte.h"
#include "common/scan.h"

/*
 * IPv6 addresses are really ugly to parse.
 * Syntax: (h = hex digit)
 *   1. hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh:hhhh
 *   2. any number of 0000 may be abbreviated as "::", but only once
 *   3. The last two words may be written as IPv4 address
 */

PRIVATE unsigned int scan_ip6(const char *src, ip6_t ip)
{
    unsigned int i;
    unsigned int len = 0;
    unsigned long u;

    uint8_t suffix[16];
    unsigned int prefixlen = 0;
    unsigned int suffixlen = 0;

    if ((i = scan_ip4(src, ip + 12))) {
        for (len = 0; len < 12; ++len) {
            ip[len] = ip6mappedv4prefix[len];
        }
        return i;
    }
    byte_zero(ip, 16);
    for (;;) {
        if (*src == ':') {
            ++len;
            ++src;
            if (*src == ':') {	/* Found "::", skip to part 2 */
	            ++len;
	            ++src;
	            break;
            }
        }
        i = scan_xlong(src, &u);
        if (!i) {
            return 0;
        }
        if (prefixlen == 12 && src[i]=='.') {
            /* the last 4 bytes may be written as IPv4 address */
            i = scan_ip4(src, ip + 12);
            if (i) {
	            return i + len;
            } else {
                return 0;
            }
        }
        ip[prefixlen++] = (u >> 8);
        ip[prefixlen++] = (u & 255);
        src += i;
        len += i;
        if (prefixlen == 16) {
            return len;
        }
    }

    /* part 2, after "::" */
    for (;;) {
        if (*src == ':') {
            if (suffixlen==0) {
                break;
            }

            src++;
            len++;
        } else if (suffixlen) {
            break;
        }
        i = scan_xlong(src, &u);
        if (!i) {
            if (suffixlen) {
                --len;
            }
            break;
        }
        if (suffixlen + prefixlen <= 12 && src[i]=='.') {
            int j = scan_ip4(src, suffix + suffixlen);
            if (j) {
                suffixlen += 4;
                len += j;
                break;
            } else {
                prefixlen = 12 - suffixlen;	/* make end-of-loop test true */
            }
        }
        suffix[suffixlen++] = (u >> 8);
        suffix[suffixlen++] = (u & 255);
        src += i;
        len += i;
        if (prefixlen + suffixlen == 16) {
            break;
        }
    }
    for (i = 0; i < suffixlen; i++) {
        ip[16 - suffixlen + i] = suffix[i];
    }
    return len;
}
