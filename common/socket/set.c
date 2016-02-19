#include <sys/socket.h>
#include "common/config.h"
#include "common/socket.h"
#include "common/platform.h"

#if defined(PLATFORM_POSIX)
#include <fcntl.h>
#include <unistd.h>
#endif

PRIVATE int socket_set_blocking(socket_t *s, int v)
{
    CHECK_SOCKET(s);
#if defined(HAVE_FCNTL_H)
    const int flags = fcntl(s->fd, F_GETFL, 0);
    if (flags == -1) {
        return -1;
    }
   
    int enabled = (flags & O_NONBLOCK) == O_NONBLOCK;
    if (enabled ^ !v) {
        return fcntl(s->fd, F_SETFL, flags ^ O_NONBLOCK);
    } else if (!enabled ^ v) {
        return fcntl(s->fd, F_SETFL, flags | O_NONBLOCK);
    } else {
        return 0;
    }
#else
    return 0;
#endif
}

PRIVATE int socket_set_ipv6only(socket_t *s, int v)
{
    CHECK_SOCKET(s);
#if defined(HAVE_SETSOCKOPT) && defined(IPV6_V6ONLY)
    return setsockopt(s->fd, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&v, sizeof(v));   
#else
    (void)v;
    return 0;
#endif
}

PRIVATE int socket_set_nopipe(socket_t *s, int v)
{
    CHECK_SOCKET(s);
#if defined(HAVE_SETSOCKOPT) && defined(SO_NOSIGPIPE)
    return setsockopt(s->fd, SOL_SOCKET, SO_NOSIGPIPE, (void *)&v, sizeof(v));   
#else
    (void)v;
    return 0;
#endif
}

PRIVATE int socket_set_reuseaddr(socket_t *s, int v)
{
    CHECK_SOCKET(s);
#if defined(HAVE_SETSOCKOPT) && defined(SO_REUSEADDR)
    return setsockopt(s->fd, SOL_SOCKET, SO_REUSEADDR, (void *)&v, sizeof(v));   
#else
    (void)v;
    return 0;
#endif
}

PRIVATE int socket_set_reuseport(socket_t *s, int v)
{
    CHECK_SOCKET(s);
#if defined(HAVE_SETSOCKOPT) && defined(SO_REUSEPORT)
    return setsockopt(s->fd, SOL_SOCKET, SO_REUSEPORT, (void *)&v, sizeof(v));   
#else
    (void)v;
    return 0;
#endif
}
