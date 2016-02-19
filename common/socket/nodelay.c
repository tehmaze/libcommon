#include <fcntl.h>
#include <sys/types.h>
#ifdef __MINGW32__
#include <winsock2.h>
#endif
#include "common/socket.h"
#include "common/platform.h"

#ifndef O_NDELAY
#define O_NDELAY O_NONBLOCK
#endif

PRIVATE int socket_nodelay(int fd)
{
#ifdef __MINGW32__
    unsigned long i = 0;
    return __winsock_errno(ioctlsocket(fd, FIONBIO, &i));
#else
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NDELAY);
#endif
}

PRIVATE int socket_nodelay_off(int fd)
{
#ifdef __MINGW32__
    unsigned long i = 1;
    return __winsock_errno(ioctlsocket(fd, FIONBIO, &i));
#else
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) & ~O_NDELAY);
#endif
}
