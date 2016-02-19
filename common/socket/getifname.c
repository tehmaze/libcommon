#include <sys/types.h>
#ifndef __MINGW32__
#include <sys/socket.h>
#include <net/if.h>
#endif
#include "common/config.h"
#include "common/socket.h"

#if defined(HAVE_IF_INDEXTONAME)
/* legacy BSD name */
#ifndef IF_NAMESIZE
#define IF_NAMESIZE IFNAMSIZ
#endif

PRIVATE static char ifname[IF_NAMESIZE];

PRIVATE const char *socket_getifname(uint32_t _interface) {
    char *tmp = if_indextoname(_interface, ifname);
    if (tmp != NULL) {
        return tmp;
    } else {
        return "[unknown]";
    }
}
#else // HAVE_IF_INDEXTONAME
PRIVATE const char *socket_getifname(uint32_t _interface)
{
    (void)_interface;
    return "[unknown]";
}
#endif // HAVE_IF_INDEXTONAME
