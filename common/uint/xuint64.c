#include "common/uint.h"

PRIVATE void xuint64_pack(uint8_t *buf, uint64_t in)
{
    xuint32_pack(buf + 0, in >> 32);
    xuint32_pack(buf + 8, in & 0xffffffffU);
}

PRIVATE void xuint64_pack_le(uint8_t *buf, uint64_t in)
{
    xuint32_pack_le(buf + 0, in & 0xffffffffU);
    xuint32_pack_le(buf + 8, in >> 32);
}
