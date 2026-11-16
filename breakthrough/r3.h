#ifndef R3_H
#define R3_H
#include "../core/wad18.h"
typedef struct { wad_t w, s, a, c; int ok_min, ok_feas, ok_range; } r3_design;
/* Exact discrete critical damping for semi-implicit Euler mass-spring-damper. */
r3_design r3_design_damper_f(wad_t m, wad_t k, wad_t dt, int *ovf);   /* reentrant */
#ifndef WAD_NO_GLOBALS
r3_design r3_design_damper(wad_t m, wad_t k, wad_t dt);                /* uses the global sticky flag */
#endif
uint64_t r3_min_s(uint64_t w);          /* smallest s with s^2 >= 4w*1e18, exact */
int r3_pred_s(uint64_t w,uint64_t s);
#endif
