#include "common/byte.h"
#include "common/uint.h"

PRIVATE void xuint16_pack(uint8_t *buf, uint16_t in)
{
    *(buf + 0) = bhex[(in >> 12) & 0x0f];
    *(buf + 1) = bhex[(in >>  8) & 0x0f];
    *(buf + 2) = bhex[(in >>  4) & 0x0f];
    *(buf + 3) = bhex[(in >>  0) & 0x0f];
}

PRIVATE void xuint16_pack_le(uint8_t *buf, uint16_t in)
{
    *(buf + 0) = bhex[(in >>  4) & 0x0f];
    *(buf + 1) = bhex[(in >>  0) & 0x0f];
    *(buf + 2) = bhex[(in >> 12) & 0x0f];
    *(buf + 3) = bhex[(in >>  8) & 0x0f];
}
