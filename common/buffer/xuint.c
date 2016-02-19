#include "common/buffer.h"
#include "common/uint.h"

#define BF(type,...)    buffer_x ## type ## __VA_ARGS__
#define BT(type)        type ## _t
#define BP(type,...)    x ## type ## _pack ## __VA_ARGS__
#define B(type,size,...) \
PRIVATE int BF(type,__VA_ARGS__)(buffer_t *buffer, BT(type) in) \
{ \
    ERROR_IF_NULL(buffer, EINVAL); \
    if (unlikely(buffer->pos + size < buffer->a)) { \
        if (buffer_grow(buffer, buffer->a + size) == -1) \
            return -1; \
    } \
    BP(type,__VA_ARGS__)((uint8_t *)(buffer->buf + buffer->pos), in); \
    return 0; \
}

B(uint8,  2)
B(uint16, 4)
B(uint16, 4, _le)
B(uint24, 6)
B(uint24, 6, _le)
B(uint32, 8)
B(uint32, 8, _le)
B(uint64, 16)
B(uint64, 16, _le)

#undef B
#undef BP
#undef BT
#undef BF
