/* WAD-18/128: signed 128-bit fixed point, scale 1e18, range ~ +/-1.7e20. Pure C99 two's-complement limbs. */
#ifndef WAD128_H
#define WAD128_H
#include "wad18.h"
typedef u128 w128;                 /* two's complement: hi = upper 64 bits */
w128 w128_from64(wad_t a);
int  w128_to64(w128 a, wad_t *out);  /* returns 0 if out of int64 range */
int  w128_cmp(w128 a, w128 b);
w128 w128_add_f(w128 a, w128 b, int *ovf);   /* reentrant API: no global state */
w128 w128_sub_f(w128 a, w128 b, int *ovf);
w128 w128_mul_f(w128 a, w128 b, int *ovf);
w128 w128_div_f(w128 a, w128 b, int *ovf);
#ifndef WAD_NO_GLOBALS
extern int wad128_overflow;                  /* sticky global flag (NOT thread/ISR safe) */
w128 w128_add(w128 a, w128 b);
w128 w128_sub(w128 a, w128 b);
w128 w128_mul(w128 a, w128 b);
w128 w128_div(w128 a, w128 b);
#endif
#endif
