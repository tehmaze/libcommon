#ifndef _COMMON_TAI_H
#define _COMMON_TAI_H

#include <inttypes.h>
#include <sys/time.h>

struct tai {
    uint64_t x;
};

#define TAI_PACK_SIZE 8

#define tai_unix(t,u) ((void) ((t)->x = 4611686018427387914ULL + (uint64_t) (u)))

void tai_now(struct tai *t);
void tai_add(struct tai *t, const struct tai *a, const struct tai *b);
void tai_sub(struct tai *t, const struct tai *a, const struct tai *b);
int  tai_cmp(const struct tai *a, const struct tai *b);
void tai_pack(const struct tai *t, char *s);
void tai_unpack(struct tai *t, const char *s);

struct taia {
    struct tai sec;
    unsigned long nano;
    unsigned long atto;
};

#define TAIA_PACK_SIZE 16

void taia_tai(const struct taia *ta, struct tai *t);
void taia_now(struct taia *ta);
void taia_add(struct taia *ta, const struct taia *a, const struct taia *b);
void taia_sub(struct taia *ta, const struct taia *a, const struct taia *b);
int  taia_cmp(const struct taia *a, const struct taia *b);
void taia_pack(const struct taia *ta, char *s);
void taia_unpack(struct taia *ta, const char *s);

#endif // _COMMON_TAI_H
