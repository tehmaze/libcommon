#include <errno.h>
#include <sys/types.h>
#if !defined(__MINGW32__)
#include <sys/socket.h>
#include <netinet/in.h>
#endif
#include "common/platform.h"
#include "common/byte.h"
#include "common/socket.h"
#include "common/uint.h"

PRIVATE int socket_connect(socket_t *s, const ip6_t ip, uint16_t port)
{
    if (s == NULL) {
        errno = EINVAL;
        return -1;
    }
    return s->v == 4
        ? socket_connect4(s->fd, ip, port)
        : socket_connect6(s->fd, ip, port, s->scope_id);
}

PRIVATE int socket_connect4(int fd, const ip4_t ip, uint16_t port)
{
    struct sockaddr_in si;
    byte_zero(&si, sizeof(si));
    si.sin_family = AF_INET;
    uint16_pack((uint8_t *)&si.sin_port, port);
    *((uint32_t *)&si.sin_addr) = *((uint32_t *)ip);
    return (__winsock_errno(connect(fd, (struct sockaddr*)&si, sizeof(si))));
}

PRIVATE int socket_connect6(int fd, const ip6_t ip, uint16_t port, uint32_t scope_id)
{
#if !defined(HAVE_LIBC_SCOPE_ID)
    (void)scope_id;
#endif
#if defined(HAVE_LIBC_IPV6)
    struct sockaddr_in6 sa;

    if (ip6disabled) {
#endif
        if (isip4mapped(ip)) {
            return __winsock_errno(socket_connect4(fd, ip + 12, port));
        }
        if (byte_equal((uint8_t *)ip, (uint8_t *)ip6loopback, 16)) {
            return __winsock_errno(socket_connect4(fd, ip4loopback, port));
        }
#if defined(HAVE_LIBC_IPV6)
    }
    byte_zero(&sa, sizeof(sa));
    sa.sin6_family = PF_INET6;
    sa.sin6_flowinfo = 0;
    uint16_pack((uint8_t *)&sa.sin6_port, port);

#if defined(HAVE_LIBC_SCOPE_ID)
    sa.sin6_scope_id = scope_id;
#endif
    byte_copy((uint8_t *)&sa.sin6_addr, (uint8_t *)ip, 16);

    return __winsock_errno(connect(fd, (void *)&sa, sizeof(sa)));
#else
    errno = EPROTONOSUPPORT;
    return -1;
#endif
}

PRIVATE bool socket_connected(socket_t *s)
{
    if (s == NULL) {
        return false;
    }
    return socket_connectedx(s->fd);
}

PRIVATE bool socket_connectedx(int fd)
{
    struct sockaddr si;
    socklen_t sl = sizeof(si);
    return getpeername(fd, &si, &sl) == 0;
}
