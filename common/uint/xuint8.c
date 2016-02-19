#include "common/byte.h"
#include "common/uint.h"

PRIVATE void xuint8_pack(uint8_t *buf, uint8_t in)
{
    *(buf + 0) = bhex[(in >>  4)];
    *(buf + 1) = bhex[(in & 0xf)];
}
