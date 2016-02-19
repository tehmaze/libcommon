#include "common/byte.h"
#include "common/uint.h"

void xuint_pack(uint8_t *buf, const uint8_t *src, size_t len)
{
    if (buf == NULL || src == NULL || len == 0)
        return;

    size_t i;
    uint8_t *dst = buf;
    for (i = 0; i < len; i++) {
        *dst++ = bhex[src[i] >> 4];
        *dst++ = bhex[src[i] & 0x0f];
    }
}

#define ST(type)        type ## _t
#define S(type,size) \
PRIVATE uint8_t *x##type##_packs(ST(type) in) \
{ \
    static uint8_t buf[size]; \
    x##type##_pack(buf, in); \
    return buf; \
}

S(uint8,  2);
S(uint16, 4);
S(uint24, 6);
S(uint32, 8);
S(uint64, 16);

#undef S
#undef ST
