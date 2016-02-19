#include "common/platform.h"

#if defined(PLATFORM_WINDOWS)
#include <stdlib.h>
#include <stdbool.h>
#include <winsock2.h>
#include <errno.h>
#include <stdio.h>

PRIVATE int __winsock_errno(long l) {
    long x;
    if (l==-1)
    switch ((x = WSAGetLastError())) {
    case WSANOTINITIALISED:
        printf("WSANOTINITIALISED!\n");
        exit(111);
    case WSAENETDOWN:
        printf("WSAENETDOWN!\n");
        exit(111);
    case WSAEINTR:
        errno = EINTR;
        break;
    case WSAEBADF:
        errno = EBADF;
        break;
    case WSAEACCES:
        errno = EACCES;
        break;
    case WSAEFAULT:
        errno = EFAULT;
        break;
    case WSAEINVAL:
        errno = EINVAL;
        break;
    case WSAEMFILE:
        errno = EMFILE;
        break;
    case WSAENAMETOOLONG:
        errno = ENAMETOOLONG;
        break;
    case WSAENOTEMPTY:
        errno = ENOTEMPTY;
        break;
    default:
        errno = x;
        break;
    }
    return l;
}

PRIVATE static bool __winsock_init_done = false;

PRIVATE void __winsock_init(void)
{
    if (__winsock_init_done) {
        return;
    }

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) ||
        LOBYTE(wsaData.wVersion) != 2 ||
        HIBYTE(wsaData.wVersion) != 2) {
        exit(111);
    }
    __winsock_init_done = true;
}

#else // PLATFORM_WINDOWS

PRIVATE void __winsock_init(void)
{
    (void)0;
}

PRIVATE int __winsock_errno(long err)
{
    return err;
}

#endif // PLATFORM_WINDOWS
