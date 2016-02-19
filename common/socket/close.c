#include <unistd.h>
#include "common/socket.h"

PRIVATE int socket_close(socket_t *s)
{
    if (s == NULL || s->fd < 0) {
        errno = EINVAL;
        return -1;
    }
    int ret = close(s->fd);
    s->fd = -1;
    return ret;
}
