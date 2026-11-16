/* R^3: Recursive Refinement, closed-form target + exact integer bisection.
 * Plant (semi-implicit Euler):  v' = v + dt(-k x - c v)/m ; x' = x + dt v'
 * With w = dt^2 k/m, a = dt c/m:  trace T=2-w-a, det D=1-a.
 * Real repeated root (fastest non-oscillatory decay) <=> T^2 = 4D <=> (a+w)^2 = 4w.
 * Closed form: a = 2*sqrt(w) - w.  We find s=a+w as the SMALLEST integer with
 * s^2 >= 4w*ONE, via recursion on an exact 128-bit predicate (no rounding in the search).
 * No mutable global or static state: the target is passed explicitly. Recursion depth <= ~60 (log2 of 1e18). */
#include "r3.h"
static int pred(uint64_t s,u128 target){ return u128_cmp(u128_mul64(s,s),target)>=0; }
static uint64_t refine(uint64_t lo,uint64_t hi,u128 target){ /* invariant: !pred(lo), pred(hi) */
    if(hi-lo<=1u){ return hi; }
    { uint64_t mid=lo+(hi-lo)/2u; return pred(mid,target)?refine(lo,mid,target):refine(mid,hi,target); } }
static u128 tgt(uint64_t w){ return u128_mul64(w,4u*(uint64_t)WAD_ONE); }   /* w*4*1e18 in 128 bits: no wrap for any w */
uint64_t r3_min_s(uint64_t w){ return refine(0u,(uint64_t)WAD_ONE,tgt(w)); }
int r3_pred_s(uint64_t w,uint64_t s){ return pred(s,tgt(w)); }
r3_design r3_design_damper_f(wad_t m,wad_t k,wad_t dt,int *ovf){
    r3_design d; wad_t kom=wad_div_f(k,m,ovf);
    d.w=wad_mul_f(wad_mul_f(dt,dt,ovf),kom,ovf);
    d.s=(wad_t)r3_min_s((uint64_t)d.w);
    d.a=wad_sub_f(d.s,d.w,ovf);
    d.c=wad_div_f(wad_mul_f(d.a,m,ovf),dt,ovf); /* order avoids m/dt overflow */
    d.ok_feas=pred((uint64_t)d.s,tgt((uint64_t)d.w));
    d.ok_min=!pred((uint64_t)d.s-1u,tgt((uint64_t)d.w));
    d.ok_range=(d.a>0 && d.a<=WAD_ONE);  /* 0 <= det=1-a < 1  => spectral radius < 1 */
    return d; }
#ifndef WAD_NO_GLOBALS
r3_design r3_design_damper(wad_t m,wad_t k,wad_t dt){ int f=0; r3_design d=r3_design_damper_f(m,k,dt,&f); if(f!=0){ wad_overflow=1; } return d; }
#endif
