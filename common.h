#ifndef _COMMON_H
#define _COMMON_H

#include <errno.h>
#include <talloc.h>
#include "common/config.h"

#if defined(__cplusplus)
extern "C" {
#endif

#if !defined(likely)
# if defined(HAVE_BUILTIN_EXPECT)
#  define likely(x) __builtin_expect(!!(x), 1)
# else
#  define likely(x) (x)
# endif
#endif

#if !defined(unlikely)
# if defined(HAVE_BUILTIN_EXPECT)
#  define unlikely(x) __builtin_expect(!!(x), 0)
# else
#  define unlikely(x) (x)
# endif
#endif

/* Branch prediction optimizations disabled */
#if defined(WITH_NO_EXPECT)
#undef  likely
#define likely(x) (x)
#undef  unlikely
#define unlikely(x) (x)
#endif

#define CHECK_IF_NULL(x,e) do { \
    if ((x) == NULL) { \
        errno = e; \
        return NULL; \
    } \
} while(0)

#define CHECK_IF_NULL_FREE(x,e,f) do { \
    if ((x) == NULL) { \
        TALLOC_FREE(f); \
        errno = e; \
        return NULL; \
    } \
} while(0)

#define ERROR_IF_NULL(x,e) do { \
    if ((x) == NULL) { \
        errno = e; \
        return -1; \
    } \
} while(0)

#define ERROR_IF_NULL_FREE(x,e,f) do { \
    if ((x) == NULL) { \
        TALLOC_FREE(f); \
        errno = e; \
        return -1; \
    } \
} while(0)

#if defined(__cplusplus)
}
#endif

#endif // _COMMON_H
