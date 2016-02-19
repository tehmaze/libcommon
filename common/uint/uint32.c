#include "common/uint.h"

PRIVATE uint32_t uint32(uint8_t *buf)
{
    return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | (buf[3]);
}

PRIVATE uint32_t uint32_le(uint8_t *buf)
{
    return (buf[0]) | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

PRIVATE void uint32_pack(uint8_t *buf, uint32_t in)
{
    buf[0] = (in >> 24) & 0xff;
    buf[1] = (in >> 16) & 0xff;
    buf[2] = (in >>  8) & 0xff;
    buf[3] = (in >>  0) & 0xff;
}

PRIVATE void uint32_pack_le(uint8_t *buf, uint32_t in)
{
    buf[0] = (in >>  0) & 0xff;
    buf[1] = (in >>  8) & 0xff;
    buf[2] = (in >> 16) & 0xff;
    buf[3] = (in >> 24) & 0xff;
}
