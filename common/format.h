#ifndef _COMMON_FORMAT_H
#define _COMMON_FORMAT_H

#include <stddef.h>
#include "common.h"
#include "common/socket.h"

#if defined(__cplusplus)
extern "C" {
#endif

#define FORMAT_IP4_LEN 	 20
#define FORMAT_IP6_LEN 	 40
#define FORMAT_ADDR4_LEN (FORMAT_IP4_LEN + 1 + 5)
#define FORMAT_ADDR6_LEN (FORMAT_IP6_LEN + 3 + 5)

unsigned int format_addr4(char *dst, const addr4_t addr);
char *       format_addr4s(const addr4_t addr);
unsigned int format_addr6(char *dst, const addr6_t addr);
char *       format_addr6s(const addr6_t addr);
unsigned int format_ip4(char *dst, const ip4_t ip);
char *       format_ip4s(const ip4_t ip);
unsigned int format_ip6(char *dst, const ip6_t ip);
char *       format_ip6s(const ip6_t ip);
int          format_path_canonical(char *dst, size_t len, const char *path);
const char * format_path_ext(const char *path);
void         format_path_join(char *dst, size_t len, const char *dir, const char *file);
char         format_path_sep(void);
size_t       format_ulong(char *dst, unsigned long i);
size_t       format_xlong(char *dst, unsigned long i);

#if defined(__cplusplus)
}
#endif

#endif // _COMMON_FORMAT_H
