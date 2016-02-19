#include "common/uint.h"

PRIVATE void xuint32_pack(uint8_t *buf, uint32_t in)
{
    xuint16_pack(buf + 0, in >> 16);
    xuint16_pack(buf + 8, in & 0xffffU);
}

PRIVATE void xuint32_pack_le(uint8_t *buf, uint32_t in)
{
    xuint16_pack_le(buf + 0, in & 0xffffU);
    xuint16_pack_le(buf + 8, in >> 16);
}
