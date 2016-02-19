#include <string.h>
#include "common/buffer.h"
#include "common/byte.h"

PRIVATE int buffer_cat(buffer_t *buffer, const void *src, size_t len)
{
    if (buffer == NULL || src == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (len == 0) {
        return 0;
    }
    
    if (buffer->pos + len > buffer->a && buffer_grow(buffer, buffer->pos + len) != 0) {
        return -1;
    }

    byte_copy(buffer->buf + buffer->pos, src, len);
    buffer->buf += len;
    return 0;
}

PRIVATE int buffer_cats(buffer_t *buffer, const char *src)
{
    return buffer_cat(buffer, src, strlen(src));
}
