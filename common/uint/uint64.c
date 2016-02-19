#include "common/uint.h"

PRIVATE uint64_t uint64(uint8_t *buf)
{
    return (((uint64_t)uint32(buf)) << 32) | (uint64_t)uint32(buf + 4);
}

PRIVATE uint64_t uint64_le(uint8_t *buf)
{
    return (uint64_t)uint32(buf) | (((uint64_t)uint32(buf + 4)) << 32);
}

PRIVATE void uint64_pack(uint8_t *buf, uint64_t in)
{
    uint32_pack(buf + 0, in >> 32);
    uint32_pack(buf + 4, in & 0xffffffffU);
}

PRIVATE void uint64_pack_le(uint8_t *buf, uint64_t in)
{
    uint32_pack_le(buf + 0, in & 0xffffffffU);
    uint32_pack_le(buf + 4, in >> 32);
}
