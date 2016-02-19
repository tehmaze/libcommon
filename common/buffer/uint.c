#include "common/buffer.h"
#include "common/byte.h"
#include "common/uint.h"

#define uint8_pack(dst,in) do { *(dst) = in; } while(0)
#define uint8_pack_le uint8_pack

#define BF(type,...)    buffer_ ## type ## __VA_ARGS__
#define BT(type)        type ## _t
#define BP(type,...)    type ## _pack ## __VA_ARGS__
#define B(type,size,...) \
PRIVATE int BF(type,__VA_ARGS__)(buffer_t *buffer, BT(type) in) \
{ \
    ERROR_IF_NULL(buffer, EINVAL); \
    if (unlikely(buffer->pos + size < buffer->a)) { \
        if (buffer_grow(buffer, buffer->a + size) == -1) \
            return -1; \
    } \
    BP(type,##__VA_ARGS__)((uint8_t *)(buffer->buf + buffer->pos), in); \
    return 0; \
}

B(uint8,  1)
B(uint16, 2)
B(uint16, 2, _le)
B(uint24, 3)
B(uint24, 3, _le)
B(uint32, 4)
B(uint32, 4, _le)
B(uint64, 8)
B(uint64, 8, _le)

#undef B
#undef BP
#undef BT
#undef BF
