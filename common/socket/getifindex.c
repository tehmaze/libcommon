#include <sys/types.h>
#if !defined(__MINGW32__)
#include <sys/socket.h>
#include <net/if.h>
#endif
#include "common/config.h"
#include "common/socket.h"

PRIVATE uint32_t socket_getifindex(const char *ifname)
{
#ifdef HAVE_IF_INDEXTONAME
    return if_nametoindex(ifname);
#else
    (void)ifname;
    return 0;
#endif
}
