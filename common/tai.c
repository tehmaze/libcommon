#include <time.h>
#include "common/tai.h"

void tai_now(struct tai *t)
{
    tai_unix(t, time((time_t *)0));
}

void tai_add(struct tai *t, const struct tai *a, const struct tai *b)
{
    t->x = a->x + b->x;
}

void tai_sub(struct tai *t, const struct tai *a, const struct tai *b)
{
    t->x = a->x - b->x;
}

int tai_cmp(const struct tai *a, const struct tai *b)
{
    if (a->x < b->x) return -1;
    else if (a->x > b->x) return +1;
    else return 0;
}

void tai_pack(const struct tai *t, char *s)
{
    uint64_t x = t->x;
    s[7] = x & 255; x >>= 8;
    s[6] = x & 255; x >>= 8;
    s[5] = x & 255; x >>= 8;
    s[4] = x & 255; x >>= 8;
    s[3] = x & 255; x >>= 8;
    s[2] = x & 255; x >>= 8;
    s[1] = x & 255; x >>= 8;
    s[0] = x;
}

void tai_unpack(struct tai *t, const char *s)
{
    uint64_t x;
    x = (unsigned char)s[0];
    x <<= 8; x += (unsigned char)s[1];
    x <<= 8; x += (unsigned char)s[2];
    x <<= 8; x += (unsigned char)s[3];
    x <<= 8; x += (unsigned char)s[4];
    x <<= 8; x += (unsigned char)s[5];
    x <<= 8; x += (unsigned char)s[6];
    x <<= 8; x += (unsigned char)s[7];
    t->x = x;
}

void taia_now(struct taia *ta)
{
    struct timeval now;
    gettimeofday(&now, (struct timezone *)0);
    ta->sec.x = 4611686018427387914ULL + (uint64_t) now.tv_sec;
    ta->nano = 1000 * now.tv_usec + 500;
    ta->atto = 0;
}

void taia_add(struct taia *ta, const struct taia *a, const struct taia *b)
{
    ta->sec.x = a->sec.x + b->sec.x;
    ta->nano = a->nano + b->nano;
    ta->atto = a->atto + b->atto;
    if (ta->atto > 999999999UL) {
        ta->atto -= 1000000000UL;
        ta->nano++;
    }
    if (ta->nano > 999999999UL) {
        ta->nano -= 1000000000UL;
        ta->sec.x++;
    }
}

void taia_sub(struct taia *ta, const struct taia *a, const struct taia *b)
{
    unsigned long anano = a->nano;
    unsigned long aatto = a->atto;

    ta->sec.x = a->sec.x - b->sec.x;
    ta->nano = anano - b->nano;
    ta->atto = aatto - b->atto;
    if (ta->atto > aatto) {
        ta->atto += 1000000000UL;
        ta->nano--;
    }
    if (ta->nano > anano) {
        ta->nano += 1000000000UL;
        ta->sec.x--;
    }
}

int taia_cmp(const struct taia *a, const struct taia *b)
{
    if      (a->sec.x < b->sec.x) return -1;
    else if (a->sec.x > b->sec.x) return +1;
    else if (a->nano  < b->nano ) return -1;
    else if (a->nano  > b->nano ) return +1;
    else if (a->atto  < b->atto ) return -1;
    else if (a->atto  > b->atto ) return +1;
    else                          return  0;
}

void taia_pack(const struct taia *ta, char *s)
{
    unsigned long x;

    tai_pack(&ta->sec, s);
    s += 8;
    x = ta->atto;
    s[7] = x & 255; x >>= 8;
    s[6] = x & 255; x >>= 8;
    s[5] = x & 255; x >>= 8;
    s[4] = x;
    x = ta->nano;
    s[3] = x & 255; x >>= 8;
    s[2] = x & 255; x >>= 8;
    s[1] = x & 255; x >>= 8;
    s[0] = x;
}

void taia_unpack(struct taia *ta, const char *s)
{
    unsigned long x;

    tai_unpack(&ta->sec, s);
    s += 8;
    x = (unsigned char)s[4];
    x <<= 8; x += (unsigned char)s[5];
    x <<= 8; x += (unsigned char)s[6];
    x <<= 8; x += (unsigned char)s[7];
    ta->atto = x;
    x = (unsigned char)s[0];
    x <<= 8; x += (unsigned char)s[1];
    x <<= 8; x += (unsigned char)s[2];
    x <<= 8; x += (unsigned char)s[3];
    ta->nano = x;
}
