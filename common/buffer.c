#include <talloc.h>
#include "common/buffer.h"
#include "common/byte.h"

buffer_t *buffer_new(size_t a)
{
    buffer_t *buffer = talloc_zero(NULL, buffer_t);
    CHECK_IF_NULL(buffer, ENOMEM);

    buffer->buf = talloc_zero_size(buffer, a);
    CHECK_IF_NULL_FREE(buffer->buf, ENOMEM, buffer);

    buffer->a = a;
    return buffer;
}

void buffer_free(buffer_t *buffer)
{
    TALLOC_FREE(buffer);
}

int buffer_reset(buffer_t *buffer)
{
    ERROR_IF_NULL(buffer, EINVAL);
    byte_zero(buffer->buf, buffer->a);
    buffer->pos = 0;
    buffer->len = 0;
    return 0;
}
