#include "common/debug.h"
#include "common/platform.h"
#include "common/socket.h"
#if defined(PLATFORM_LINUX)
#include <sys/sendfile.h>
#include <unistd.h>
#elif defined(PLATFORM_DARWIN)
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#endif

PRIVATE ssize_t socket_sendfile(socket_t *s, int in, off_t offset, size_t len)
{
    return socket_sendfilex(s->fd, in, offset, len);
}

PRIVATE ssize_t socket_sendfilex(int fd, int in, off_t offset, size_t len)
{
#if defined(PLATFORM_LINUX)
    off_t off = offset;
    return sendfile(fd, in, &off, len);
#elif defined(PLATFORM_DARWIN)
    off_t siz = len;
    return (ssize_t)sendfile(in, fd, offset, &siz, NULL, 0);
#else
    errno = ENOTSUP;
    return -1;
#endif
}

PRIVATE int socket_sendfile_full(socket_t *s, int in, size_t len, size_t *out)
{
    if (s == NULL) {
        errno = EINVAL;
        return -1;
    }
    return socket_sendfilex_full(s->fd, in, len, out);
}

/* Retry, if:
 * EINTR      A signal interrupted sendfile() before it could be completed.
 * EAGIN      The socket is marked for non-blocking I/O and not all data was
 *            sent due to the socket buffer being full.
 */
#define SENDFILE_CAN_RETRY      (errno == EINTR   || errno == EAGAIN)

/* Not supported, if:
 * ENOTSUP    The in argument does not refer to a regular file.
 * ENOTSOCK   The fd argument does not refer stream oriented socket.
 * EOPNOTSUPP The file system for descriptor in does not support sendfile()
 */
#define SENDFILE_NOT_SUPPORTED  (errno == ENOTSUP || errno == ENOTSOCK || errno == EOPNOTSUPP)

PRIVATE int socket_sendfilex_full(int fd, int in, size_t len, size_t *out)
{
    off_t offset;
    register ssize_t left;
    register ssize_t ret;

    left = len;
    DEBUGF("sendfile: %llu bytes", left);
    while (left > 0) {
        offset = len - (size_t)left;
        ret = socket_sendfilex(fd, in, offset, left);
        if (ret == 0) {
            // EOF
            break;
        } else if (ret < 0) {
            if (SENDFILE_CAN_RETRY) {
                continue;
            } else if (SENDFILE_NOT_SUPPORTED) {
                break;
            }
            if (out != NULL) {
                *out = left;
            }
            return -1;
        }
        left -= ret;
    }
    if (left > 0) {
        DEBUGF("sendfile: %lld bytes left, trying regular copy", left);
        char buf[128];
        ssize_t chunk;
        while (left > 0) {
            do {
                ret = read(in, buf, sizeof buf);
            } while (ret == -1 && errno == EINTR);
            if (ret == -1) {
                DEBUGF("sendfile: read(): %s", strerror(errno));
                if (out != NULL) {
                    *out = left;
                }
                return -1;
            }
            chunk = ret;
            left -= ret;
            DEBUGF("sendfile: write(): %lld bytes", chunk);
            errno = 0;
            do {
                if (errno != 0) {
                    DEBUGF("sendfile: write() again: %s", strerror(errno));
                }
                ret = write(fd, buf, chunk);
            } while (ret == -1 && (errno == EINTR || errno == EWOULDBLOCK));
            if (ret == -1) {
                DEBUGF("sendfile: write(): %s", strerror(errno));
                if (out != NULL) {
                    *out = left;
                }
                return -1;
            }
        }
    }
    if (out != NULL) {
        *out = left;
    }
    return 0;
}
