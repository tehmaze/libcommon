#include <errno.h>
#include <sys/types.h>
#if !defined(__MINGW32__)
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include "common/platform.h"
#include "common/socket.h"

PRIVATE socket_t *socket_tcp4(void)
{
    socket_t *s = socket_new4();
    if (s == NULL) {
        errno = EINVAL;
        return NULL;
    }
    __winsock_init();
    s->fd = __winsock_errno(socket(PF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (s->fd == -1) {
        socket_free(s);
        return NULL;
    }
    return s;
}

PRIVATE socket_t *socket_tcp6(uint32_t scope_id)
{
    socket_t *s = socket_new6(scope_id);
    if (s == NULL) {
        return NULL;
    }
#if defined(HAVE_LIBC_IPV6)
    __winsock_init();
    if (ip6disabled) {
        goto compat;
    }
    s->fd = __winsock_errno(socket(PF_INET6, SOCK_STREAM, 0));
    if (s->fd == -1) {
        if (errno == EINVAL ||
            errno == EAFNOSUPPORT ||
            errno == EPFNOSUPPORT ||
            errno == EPROTONOSUPPORT) {
compat:
            s->fd = __winsock_errno(socket(AF_INET, SOCK_STREAM, 0));
            ip6disabled = true;
            if (s->fd == -1) {
                socket_free(s);
                return NULL;
            }
        } else {
            socket_free(s);
            return NULL;
        }
    }
#ifdef IPV6_V6ONLY
    {
        int zero=0;
        __winsock_errno(setsockopt(s->fd, IPPROTO_IPV6, IPV6_V6ONLY, (void*)&zero, sizeof(zero)));
    }
#endif
    return s;
#else
    return socket_tcp4();
#endif
}
