#include <unistd.h>
#include "common/socket.h"

PRIVATE ssize_t socket_read(socket_t *s, void *buf, size_t len)
{
    if (s == NULL) {
        errno = EINVAL;
        return -1;
    }
    return read(s->fd, buf, len);
}
