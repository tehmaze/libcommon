#include <errno.h>
#include <sys/types.h>
#if !defined(__MINGW32__)
#include <sys/socket.h>
#endif
#include "common/socket.h"

PRIVATE int socket_listen(socket_t *s, unsigned int backlog)
{
    if (s == NULL) {
        errno = EINVAL;
        return -1;
    }
    return socket_listenx(s->fd, backlog);
}

PRIVATE int socket_listenx(int fd, unsigned int backlog)
{
    return listen(fd, backlog);
}
