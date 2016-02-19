#include "common/byte.h"
#include "common/uint.h"

PRIVATE void xuint24_pack(uint8_t *buf, uint24_t in)
{
    *(buf + 0) = bhex[(in >> 20) & 0x0f];
    *(buf + 1) = bhex[(in >> 16) & 0x0f];
    *(buf + 2) = bhex[(in >> 12) & 0x0f];
    *(buf + 3) = bhex[(in >>  8) & 0x0f];
    *(buf + 4) = bhex[(in >>  4) & 0x0f];
    *(buf + 5) = bhex[(in >>  0) & 0x0f];
}

PRIVATE void xuint24_pack_le(uint8_t *buf, uint24_t in)
{
    *(buf + 0) = bhex[(in >>  0) & 0x0f];
    *(buf + 1) = bhex[(in >>  4) & 0x0f];
    *(buf + 2) = bhex[(in >> 12) & 0x0f];
    *(buf + 3) = bhex[(in >>  8) & 0x0f];
    *(buf + 4) = bhex[(in >> 20) & 0x0f];
    *(buf + 5) = bhex[(in >> 16) & 0x0f];
}
