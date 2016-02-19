#include <errno.h>
#include <sys/types.h>
#if defined(__MINGW32__)
#include <io.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include "common/config.h"
#include "common/socket.h"
#include "common/platform.h"

PRIVATE socket_t *socket_udp4(void)
{
    socket_t *s = socket_new4();
    if (s == NULL) {
        errno = EINVAL;
        return NULL;
    }
    __winsock_init();
    s->fd = __winsock_errno(socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP));
    if (s->fd == -1) {
        socket_free(s);
        return NULL;
    }
    if (socket_nodelay(s->fd) == -1) {
        close(s->fd);
        socket_free(s);
        return NULL;
    }
    return s;
}

PRIVATE socket_t *socket_udp6(uint32_t scope_id)
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
    s->fd = __winsock_errno(socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP));
    if (s->fd == -1) {
        if (errno == EINVAL ||
            errno == EAFNOSUPPORT ||
            errno == EPFNOSUPPORT ||
            errno == EPROTONOSUPPORT) {
compat:
            s->fd = __winsock_errno(socket(AF_INET, SOCK_DGRAM, 0));
            ip6disabled = true;
            if (s->fd == -1) {
                socket_free(s);
                return NULL;
            }
        }
        if (s->fd == -1) {
            socket_free(s);
            return NULL;
        }
        return s;
    }
#if defined(IPV6_V6ONLY)
    int zero = 0;
    __winsock_errno(setsockopt(s->fd, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&zero, sizeof(zero)));
#endif
    if (socket_nodelay(s->fd) == -1) {
        close(s->fd);
        socket_free(s);
        return NULL;
    }
    return s;
#else // HAVE_LIBC_IPV6
    return s;
#endif
}
