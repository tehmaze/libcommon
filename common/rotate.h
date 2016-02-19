/* rotate.h -- Rotate functions
2009-02-07 : Igor Pavlov : Public domain */

#ifndef __ROTATE_DEFS_H
#define __ROTATE_DEFS_H

#if defined(__cplusplus)
extern "C" {
#endif

#ifdef _MSC_VER

#include <stdlib.h>
#define rotl_fixed(x, n) _rotl((x), (n))
#define rotr_fixed(x, n) _rotr((x), (n))

#else

#define rotl_fixed(x, n) (((x) << (n)) | ((x) >> (32 - (n))))
#define rotr_fixed(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

#endif

#if defined(__cplusplus)
}
#endif

#endif
