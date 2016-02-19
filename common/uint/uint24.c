#include "common/uint.h"

PRIVATE uint24_t uint24(uint8_t *buf)
{
    return (buf[0] << 16) | (buf[1] << 8) | (buf[2]);
}

PRIVATE uint24_t uint24_le(uint8_t *buf)
{
    return (buf[0]) | (buf[1] << 8) | (buf[2] << 16);
}

PRIVATE void uint24_pack(uint8_t *buf, uint24_t in)
{
    buf[0] = (in >> 16) & 0xff;
    buf[1] = (in >>  8) & 0xff;
    buf[2] = (in >>  0) & 0xff;
}

PRIVATE void uint24_pack_le(uint8_t *buf, uint24_t in)
{
    buf[0] = (in >>  0) & 0xff;
    buf[1] = (in >>  8) & 0xff;
    buf[2] = (in >> 16) & 0xff;
}
