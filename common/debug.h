#ifndef _COMMON_DEBUG_H
#define _COMMON_DEBUG_H

#include <errno.h>
#include <stddef.h>
#include <string.h>
#include "common.h"
#include "common/platform.h"
#include "common/format.h"
#include "common/scan.h"
#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

extern void (*debug_handler)(const char *fmt, ...);

#if !defined(OK)
#define OK 0
#endif

#if !defined(ENOTSUP)
#if defined(EOPNOTSUPP)
#define ENOTSUP EOPNOTSUPP
#else
#define ENOTSUP 4096
#endif
#endif

#define DEBUGF(fmt, ...)      do { \
    if (debug_handler) \
        debug_handler("%s[%d](%s): " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
} while(0)
#define DEBUG(msg)            DEBUGF(msg)
#define DEBUG_ERROR(err, msg) DEBUGF("%s returning " #err ": " msg, __func__)
#define RETURN_CODE(x) do { \
	DEBUGF("%s returning " #x, __func__); \
	return x; \
} while (0)
#define RETURN_CODEVAL(x) do { \
	switch (x) { \
	case OK:      RETURN_CODE(OK); \
	case EINVAL:  RETURN_CODE(EINVAL); \
	case ENOMEM:  RETURN_CODE(ENOMEM); \
	case ENOTSUP: RETURN_CODE(ENOTSUP); \
	default:      RETURN_CODE(EINVAL); \
	} \
} while (0)
#define RETURN_OK()           RETURN_CODE(OK)
#define RETURN_ERRNO(msg, ...) do { \
	DEBUGF("%s returning %s: " msg, __func__, strerror(errno), ##__VA_ARGS__); \
	return errno; \
} while (0)
#define RETURN_ERROR(err, msg) do { \
	errno = err; \
	DEBUG_ERROR(err, msg); \
	return err; \
} while (0)
#define RETURN_FAIL(msg)      DEBUG(msg); return -1
#define RETURN_FAILF(msg,...) DEBUGF(msg,__VA_ARGS__); return -1
#define RETURN_INT(i)         do { \
	int _i = i; \
	DEBUGF("%s: returning %d", __func__, _i); \
	return _i; \
} while (0)
#define RETURN_STRING(s)      do { \
    char *_s = s; \
    DEBUGF("%s: returning %s", __func__, _s); \
    return _s; \
} while(0)
#define SET_ERROR(val, err, msg) do { \
    errno = err; \
    DEBUG_ERROR(err, msg); \
    val = err; \
} while (0)
#define SET_FAIL(ret, msg)    do { \
    ret = -1; \
    DEBUG(msg); \
} while(0)
#define TRY(x) do { \
    int retval = x; \
    if (retval != 0) RETURN_CODEVAL(retval); \
} while (0)

char *debugip4(ip4_t ip);
char *debugip6(ip6_t ip);

#if defined(__cplusplus)
}
#endif

#endif // _COMMON_DEBUG_H
