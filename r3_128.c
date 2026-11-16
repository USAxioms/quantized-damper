/* R^3 over 128-bit WAD: same exact criterion (a+w)^2=4w. w=dt^2 k/m must lie in (0,1) for stability, so s fits 64 bits
 * even when m, k, c are huge; only c = a*m/dt needs the wide type. No mutable global/static state in the _f form. */
#include "r3_128.h"
#include "r3.h"
r3_design128 r3_design_damper128_f(w128 m,w128 k,w128 dt,int *ovf){
    r3_design128 d; w128 w128v=w128_mul_f(w128_mul_f(dt,dt,ovf),w128_div_f(k,m,ovf),ovf); wad_t w64=0; w128 a128;
    d.c=w128_from64(0); d.s=0; d.w=0;
    d.ok_domain=w128_to64(w128v,&w64)&&w64>0&&w64<WAD_ONE;
    if(!d.ok_domain){ d.ok_feas=d.ok_min=d.ok_range=0; return d; }
    d.w=w64; d.s=(wad_t)r3_min_s((uint64_t)w64);
    d.ok_feas=r3_pred_s((uint64_t)w64,(uint64_t)d.s);
    d.ok_min=!r3_pred_s((uint64_t)w64,(uint64_t)d.s-1u);
    a128=w128_from64(d.s-d.w); d.ok_range=(d.s-d.w>0&&d.s-d.w<=WAD_ONE);
    d.c=w128_div_f(w128_mul_f(a128,m,ovf),dt,ovf); return d; }
#ifndef WAD_NO_GLOBALS
r3_design128 r3_design_damper128(w128 m,w128 k,w128 dt){ int f=0; r3_design128 d=r3_design_damper128_f(m,k,dt,&f); if(f!=0){ wad128_overflow=1; } return d; }
#endif
