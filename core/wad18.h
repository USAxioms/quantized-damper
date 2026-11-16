/* WAD-18: signed 64-bit fixed point, scale 10^18. Range ~ +/-9.22. Pure integer, C99, no libs. */
#ifndef WAD18_H
#define WAD18_H
#include <stdint.h>
typedef int64_t wad_t;
#define WAD_ONE ((wad_t)1000000000000000000LL)
typedef struct { uint64_t hi, lo; } u128;
u128 u128_mul64(uint64_t a, uint64_t b); /* exact 64x64 -> 128 */
int  u128_cmp(u128 a, u128 b);
u128 wad_udivround(u128 n, uint64_t d); /* round-half-up quotient n/d (d != 0) */
/* Reentrant API: no global state. *ovf is set to 1 if the result saturated (never cleared by the callee). */
wad_t wad_add_f(wad_t a, wad_t b, int *ovf);
wad_t wad_sub_f(wad_t a, wad_t b, int *ovf);
wad_t wad_mul_f(wad_t a, wad_t b, int *ovf);   /* round half away from zero, saturating */
wad_t wad_div_f(wad_t a, wad_t b, int *ovf);
#ifndef WAD_NO_GLOBALS
extern int wad_overflow;                 /* sticky global flag: set if any op saturated (NOT thread/ISR safe) */
wad_t wad_add(wad_t a, wad_t b);
wad_t wad_sub(wad_t a, wad_t b);
wad_t wad_mul(wad_t a, wad_t b);
wad_t wad_div(wad_t a, wad_t b);
#endif
#endif
