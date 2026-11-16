/* Report-only timing (host-dependent; NOT part of gates/seal). POSIX clock used only here; core stays libc-free.
 * Both loops are driven by the same xorshift excitation so the double state never decays into subnormals. */
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <time.h>
#include "../core/wad18.h"
static unsigned int rs;
static unsigned int rnd(void){ rs^=rs<<13; rs^=rs>>17; rs^=rs<<5; return rs; }
static double now(void){ struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return (double)t.tv_sec+(double)t.tv_nsec*1e-9; }
int main(void){
    const long N=3000000; long i; volatile wad_t sink=0; volatile double dsink=0;
    const wad_t k=4*WAD_ONE,c=3996000000000000000LL,dt=WAD_ONE/1000; wad_t x=0,v=0; double fx=0.0,fv=0.0; double t0,t1,t2,t3;
    rs=2463534242u; t0=now();
    for(i=0;i<N;i++){ wad_t f=((wad_t)(rnd()&0xFFFFu)-32768)*152587890625LL;
        wad_t a=wad_div(wad_add(f,wad_add(wad_sub(0,wad_mul(k,x)),wad_sub(0,wad_mul(c,v)))),WAD_ONE);
        v=wad_add(v,wad_mul(dt,a)); x=wad_add(x,wad_mul(dt,v)); }
    t1=now(); sink=x;
    rs=2463534242u;
    for(i=0;i<N;i++){ double f=((double)(rnd()&0xFFFFu)-32768.0)*(0.01/65536.0); double a=f-4.0*fx-3.996*fv; fv+=0.001*a; fx+=0.001*fv; }
    t2=now(); dsink=fx;
    { wad_t a=123456789012345678LL,b=987654321098765432LL,acc=0; for(i=0;i<N;i++){ acc^=wad_mul(a+i,b); } sink=acc; }
    t3=now();
    printf("WAD-18 control step (1 div + 4 mul + adds + excitation): %.1f ns/step over %ld steps\n",(t1-t0)/(double)N*1e9,N);
    printf("double control step (same plant, same excitation):       %.2f ns/step\n",(t2-t1)/(double)N*1e9);
    printf("wad_mul alone: %.1f ns/op\n",(t3-t2)/(double)N*1e9);
    printf("ratio WAD/double = %.0fx on this host (x86-64, -O2)\n",(t1-t0)/(t2-t1));
    printf("timing bound: wide division is word-wise: <=4 digit steps (short path) or 3 digit steps x <=2 qhat corrections (two-digit path); bounded and small, but data-dependent, NOT constant-time\n");
    printf("(sinks %lld %g)\n",(long long)sink,(double)dsink);
    return 0; }
