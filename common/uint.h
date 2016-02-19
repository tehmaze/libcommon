#ifndef _COMMON_UINT_H
#define _COMMON_UINT_H

#include <inttypes.h>
#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __uint24_t
#define __uint24_t __uint32_t
#define uint24_t   uint32_t
#endif // __uint24_t

uint16_t   uint16(uint8_t *buf);
uint16_t   uint16_le(uint8_t *buf);
uint24_t   uint24(uint8_t *buf);
uint24_t   uint24_le(uint8_t *buf);
uint32_t   uint32(uint8_t *buf);
uint32_t   uint32_le(uint8_t *buf);
uint64_t   uint64(uint8_t *buf);
uint64_t   uint64_le(uint8_t *buf);
void       uint8_pack(uint8_t *buf, uint8_t in);
#define    uint8_pack_le uint8_pack
uint8_t *  uint8_packs(uint8_t in);
#define    uint8_packs_le uint8_packs
void       uint16_pack(uint8_t *buf, uint16_t in);
void       uint16_pack_le(uint8_t *buf, uint16_t in);
uint8_t *  uint16_packs(uint16_t in);
uint8_t *  uint16_packs_le(uint16_t in);
void       uint24_pack(uint8_t *buf, uint24_t in);
void       uint24_pack_le(uint8_t *buf, uint24_t in);
uint8_t *  uint24_packs(uint24_t in);
uint8_t *  uint24_packs_le(uint24_t in);
void       uint32_pack(uint8_t *buf, uint32_t in);
void       uint32_pack_le(uint8_t *buf, uint32_t in);
uint8_t *  uint32_packs(uint32_t in);
uint8_t *  uint32_packs_le(uint32_t in);
void       uint64_pack(uint8_t *buf, uint64_t in);
void       uint64_pack_le(uint8_t *buf, uint64_t in);
uint8_t *  uint64_packs(uint64_t in);
uint8_t *  uint64_packs_le(uint64_t in);
void       xuint_pack(uint8_t *buf, const uint8_t *src, size_t len);
void       xuint8_pack(uint8_t *buf, uint8_t in);
#define    xuint8_pack_le xuint8_pack
uint8_t *  xuint8_packs(uint8_t in);
#define    xuint8_packs_le xuint8_packs
void       xuint16_pack(uint8_t *buf, uint16_t in);
void       xuint16_pack_le(uint8_t *buf, uint16_t in);
uint8_t *  xuint16_packs(uint16_t in);
uint8_t *  xuint16_packs_le(uint16_t in);
void       xuint24_pack(uint8_t *buf, uint24_t in);
void       xuint24_pack_le(uint8_t *buf, uint24_t in);
uint8_t *  xuint24_packs(uint24_t in);
uint8_t *  xuint24_packs_le(uint24_t in);
void       xuint32_pack(uint8_t *buf, uint32_t in);
void       xuint32_pack_le(uint8_t *buf, uint32_t in);
uint8_t *  xuint32_packs(uint32_t in);
uint8_t *  xuint32_packs_le(uint32_t in);
void       xuint64_pack(uint8_t *buf, uint64_t in);
void       xuint64_pack_le(uint8_t *buf, uint64_t in);
uint8_t *  xuint64_packs(uint64_t in);
uint8_t *  xuint64_packs_le(uint64_t in);

#ifdef __cplusplus
}
#endif

#endif // _COMMON_UINT_H
