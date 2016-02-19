#include "common/byte.h"
#include "common/config.h"
#include "common/debug.h"
#include "common/format.h"
#include "common/socket.h"

#if defined(HAVE_ARPA_INET_H) && defined(HAVE_NETDB_H)
#include <arpa/inet.h>
#include <netdb.h>

int ip6resolve(ip6_t ip, const char *host)
{
    struct addrinfo hints, *ai = NULL, *ai0 = NULL;
    byte_zero(&hints, sizeof hints);
    hints.ai_flags = AI_ADDRCONFIG;
    
    int ret;
    if ((ret = getaddrinfo(host, NULL, &hints, &ai0)) != 0)
        return ret;

    ret = 1;
    errno = EFAULT;
    byte_zero(ip, sizeof(ip6_t));
    for (ai = ai0; ai->ai_next != NULL; ai = ai->ai_next) {
        /*
        switch (ai->ai_family) {
        case AF_INET:
            DEBUGF("getaddrinfo result: family=AF_INET, len=%lld", ai->ai_addrlen);
            byte_copy(ip, ip6mappedv4prefix, sizeof ip6mappedv4prefix);
            byte_copy(ip + sizeof(ip6mappedv4prefix), &((struct sockaddr_in *)ai->ai_addr)->sin_addr, 4);
            ret = 0;
            goto done;

        case AF_INET6:
            DEBUGF("getaddrinfo result: family=AF_INET6, len=%lld", ai->ai_addrlen);
            byte_copy(ip, &((struct sockaddr_in6 *)ai->ai_addr)->sin6_addr, 16);
            ret = 0;
            goto done;

        default:
            DEBUGF("getaddrinfo result: family=%d, len=%lld", ai->ai_family, ai->ai_addrlen);
            break;
        }
        */
        if (ai->ai_family == AF_INET) {
            struct sockaddr_in *sa = (struct sockaddr_in *)ai->ai_addr;
            byte_copy(ip, ip6mappedv4prefix, 12);
            byte_copy(ip + 12, &sa->sin_addr, sizeof(struct in_addr));
            ret = 0;
            break;
        } else if (ai->ai_family == AF_INET6) {
            struct sockaddr_in6 *sa = (struct sockaddr_in6 *)ai->ai_addr;
            byte_copy(ip, &sa->sin6_addr, sizeof(struct in6_addr));
            ret = 0;
            break;
        } else {
            DEBUGF("getaddrinfo result: unsupported family %d", ai->ai_family);
            continue;
        }
    }

    DEBUGF("%s resolved to %s", host, format_ip6s(ip));
    freeaddrinfo(ai0);
    return ret;
}
#else
int ip6resolve(ip6_t ip, const char *host)
{
    (void)ip;
    (void)host;
    return -1;  // TODO(maze): port
}
#endif
