#ifndef R3_128_H
#define R3_128_H
#include "../core/wad128.h"
typedef struct { w128 c; wad_t w, s; int ok_domain, ok_feas, ok_min, ok_range; } r3_design128;
r3_design128 r3_design_damper128_f(w128 m, w128 k, w128 dt, int *ovf);   /* reentrant */
#ifndef WAD_NO_GLOBALS
r3_design128 r3_design_damper128(w128 m, w128 k, w128 dt);
#endif
#endif
