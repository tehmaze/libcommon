#include <sys/socket.h>
#include <unistd.h>
#include "common/socket.h"

PRIVATE ssize_t socket_write(socket_t *s, const void *buf, size_t len)
{
    if (s == NULL) {
        errno = EINVAL;
        return -1;
    }
#if defined(MSG_NOSIGNAL)
    return send(s->fd, buf, len, MSG_NOSIGNAL);
#else
    return write(s->fd, buf, len);
#endif
}
