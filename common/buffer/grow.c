#include <errno.h>
#include <talloc.h>
#include "common/buffer.h"

PRIVATE int buffer_grow(buffer_t *buffer, size_t len)
{
    if (buffer == NULL) {
        errno = EINVAL;
        return -1;
    }
    if (buffer->a >= len) {
        return 0;
    }

    char *tmp = talloc_realloc(buffer, buffer->buf, char, len);
    if (tmp == NULL) {
        return -1;
    }
    buffer->buf = tmp;
    return 0;
}
