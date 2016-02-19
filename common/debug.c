#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "common/config.h"
#include "common/debug.h"

PRIVATE static void debug_handler_default(const char *fmt, ...)
{
    va_list args;
	va_start(args, fmt);
	if (getenv("COMMON_DEBUG")) {
		vfprintf(stderr, fmt, args);
	}
	va_end(args);
}

PRIVATE void (*debug_handler)(const char *fmt, ...) = debug_handler_default;

PRIVATE char *debugip4(ip4_t ip)
{
    static char host[FORMAT_IP4_LEN];
    format_ip4(host, ip);
    return host;
}

PRIVATE char *debugip6(ip6_t ip)
{
    static char host[FORMAT_IP6_LEN];
    format_ip6(host, ip);
    return host;
}
