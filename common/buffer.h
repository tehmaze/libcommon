#ifndef _COMMON_BUFFER_H
#define _COMMON_BUFFER_H

#include <stddef.h>
#include "common.h"
#include "common/uint.h"

typedef struct {
    char   *buf;       /* actual buffer */
    size_t pos;        /* current position */
    size_t len;        /* string length */
    size_t a;          /* allocated buffer size */
} buffer_t;

buffer_t * buffer_new(size_t a);
void       buffer_free(buffer_t *buffer);
int        buffer_reset(buffer_t *buffer);
int        buffer_grow(buffer_t *buffer, size_t len);
int        buffer_cat(buffer_t *buffer, const void *src, size_t len);
int        buffer_cats(buffer_t *buffer, const char *src);
int        buffer_uint8(buffer_t *buffer, uint8_t in);
int        buffer_uint16(buffer_t *buffer, uint16_t in);
int        buffer_uint16_le(buffer_t *buffer, uint16_t in);
int        buffer_uint24(buffer_t *buffer, uint24_t in);
int        buffer_uint24_le(buffer_t *buffer, uint24_t in);
int        buffer_uint32(buffer_t *buffer, uint32_t in);
int        buffer_uint32_le(buffer_t *buffer, uint32_t in);
int        buffer_uint64(buffer_t *buffer, uint64_t in);
int        buffer_uint64_le(buffer_t *buffer, uint64_t in);
int        buffer_xuint8(buffer_t *buffer, uint8_t in);
int        buffer_xuint16(buffer_t *buffer, uint16_t in);
int        buffer_xuint16_le(buffer_t *buffer, uint16_t in);
int        buffer_xuint24(buffer_t *buffer, uint24_t in);
int        buffer_xuint24_le(buffer_t *buffer, uint24_t in);
int        buffer_xuint32(buffer_t *buffer, uint32_t in);
int        buffer_xuint32_le(buffer_t *buffer, uint32_t in);
int        buffer_xuint64(buffer_t *buffer, uint64_t in);
int        buffer_xuint64_le(buffer_t *buffer, uint64_t in);

#endif // _COMMON_BUFFER_H
