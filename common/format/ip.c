#include "common/format.h"
#include "common/socket.h"

PRIVATE unsigned int format_addr4(char *dst, const addr4_t addr)
{
    unsigned int len;
    if ((len = format_ip4(dst, addr.ip)) == 0)
        return 0;

    dst[len++] = ':';
    len += format_ulong(dst, addr.port);
    return len;
}

PRIVATE char *format_addr4s(const addr4_t addr)
{
    static char s[FORMAT_ADDR4_LEN];
    byte_zero(s, sizeof s);
    format_addr4(s, addr);
    return s;
}

PRIVATE unsigned int format_addr6(char *dst, const addr6_t addr)
{
    unsigned int len;
    if ((len = format_ip6(dst + 1, addr.ip)) == 0)
        return 0;

    len++;
    dst[0] = '[';
    dst[len++] = ']';
    dst[len++] = ':';
    len += format_ulong(dst, addr.port);
    return len;
}

PRIVATE char *format_addr6s(const addr6_t addr)
{
    static char s[FORMAT_ADDR6_LEN];
    byte_zero(s, sizeof s);
    format_addr6(s, addr);
    return s;
}

PRIVATE unsigned int format_ip4(char *dst, const ip4_t ip)
{
    unsigned int len;
    int i;

    len = 0;
    for (i = 0; i < 4; ++i) {
        register unsigned int j;
        len += (j = format_ulong(dst, (unsigned long)(unsigned char)ip[i])) + 1;
        if (dst && i < 3) {
            dst += j;
            *dst++ = '.';
        }
    }
    return len - 1;
}

PRIVATE char *format_ip4s(const ip4_t ip)
{
    static char s[FORMAT_IP4_LEN];
    byte_zero(s, sizeof s);
    format_ip4(s, ip);
    return s;
}

PRIVATE unsigned int format_ip6(char *dst, const ip6_t ip)
{
    unsigned long len, temp, k, pos0 = 0, len0 = 0, pos1 = 0, compr = 0;

    for (k = 0; k < 16; k += 2) {
        if (ip[k] == 0 && ip[k + 1] == 0) {
            if (!compr) {
                compr = 1;
                pos1 = k;
            }
            if (k == 14) {
                k = 16;
                goto last;
            }
        } else if (compr) {
last:
            if ((temp = k - pos1) > len0) {
                len0 = temp;
                pos0 = pos1;
            }
            compr = 0;
        }
    }

    for (len = 0, k = 0; k < 16; k += 2) {
        if (k == 12 && isip4mapped(ip)) {
            len += format_ip4(dst, ip+12);
            break;
        }
        if (pos0 == k && len0) {
            if (k == 0) {
                ++len;
                if (dst) {
                    *dst++ = ':';
                }
            }
            ++len;
            if (dst) {
                *dst++ = ':';
            }
            k += len0-2;
            continue;
        }
        temp = ((unsigned long) (unsigned char) ip[k] << 8) +
                (unsigned long) (unsigned char) ip[k + 1];
        temp = format_xlong(dst, temp);
        len += temp;
        if (dst) {
            dst += temp;
        }
        if (k < 14) {
            ++len;
            if (dst) {
                *dst++ = ':';
            }
        }
    }
    return len;
}

PRIVATE char *format_ip6s(const ip6_t ip)
{
    static char s[FORMAT_IP6_LEN];
    byte_zero(s, sizeof s);
    format_ip6(s, ip);
    return s;
}
