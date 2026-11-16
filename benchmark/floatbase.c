/* Float baseline: same plant/disturbance/seed as bench.c, in double AND WAD-18, side by side. */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "../core/wad18.h"
#include "../core/sha256.h"
#include "../breakthrough/r3.h"
#define CYCLES 1000000
static uint32_t xs=2463534242u;
static uint32_t rnd(void){ xs^=xs<<13; xs^=xs>>17; xs^=xs<<5; return xs; }
static void put_d(sha256_ctx*h,double d){ uint64_t u; memcpy(&u,&d,8); sha256_put_i64(h,(int64_t)u); }
int main(void){
    wad_t m=WAD_ONE,k=4*WAD_ONE,dt=WAD_ONE/1000; r3_design d=r3_design_damper(m,k,dt);
    wad_t wx=0,wv=0; double fx=0.0,fv=0.0; const double fdt=0.001,fk=4.0,fc=3.996,fm=1.0;
    long double lx=0.0L,lv=0.0L; double mdl=0.0,mwl=0.0; sha256_ctx hw,hf; uint8_t dg[32]; char hx[65]; double maxd=0.0,sumsq=0.0,maxx=0.0; int i;
    sha256_init(&hw); sha256_init(&hf);
    for(i=0;i<CYCLES;i++){
        uint32_t n=rnd()&0xFFFFu;
        wad_t f=((wad_t)n-32768)*152587890625LL;
        wad_t a=wad_div(wad_add(f,wad_add(wad_mul(wad_sub(0,k),wx),wad_mul(wad_sub(0,d.c),wv))),m);
        wv=wad_add(wv,wad_mul(dt,a)); wx=wad_add(wx,wad_mul(dt,wv)); sha256_put_i64(&hw,wx); sha256_put_i64(&hw,wv);
        { double ff=((double)n-32768.0)*(0.01/65536.0); double fa=(ff-fk*fx-fc*fv)/fm;
          fv=fv+fdt*fa; fx=fx+fdt*fv; put_d(&hf,fx); put_d(&hf,fv);
          { long double lf=((long double)n-32768.0L)*(152587890625.0L/1e18L); long double la=(lf-4.0L*lx-3.996L*lv)/1.0L;
          double dl,wl; lv=lv+0.001L*la; lx=lx+0.001L*lv;
          dl=(double)fabsl((long double)fx-lx); wl=(double)fabsl((long double)wx/1e18L-lx); if(dl>mdl)mdl=dl; if(wl>mwl)mwl=wl; }
        { double diff=fabs(fx-(double)wx/1e18); if(diff>maxd)maxd=diff; sumsq+=diff*diff; if(fabs(fx)>maxx)maxx=fabs(fx); } } }
    sha256_final(&hw,dg); sha256_hex(dg,hx); printf("WAD_TRACE_SHA256 %s\n",hx);
    sha256_final(&hf,dg); sha256_hex(dg,hx); printf("DOUBLE_TRACE_SHA256 %s\n",hx);
    printf("max_abs_diff_double_vs_wad=%.3e rms_diff=%.3e max_abs_x=%.3e\n",maxd,sqrt(sumsq/CYCLES),maxx);
    printf("REF long double: max_abs_err_double=%.3e max_abs_err_wad=%.3e\n",mdl,mwl);
    printf("final_x_double=%.17g final_x_wad=%.17g\n",fx,(double)wx/1e18);
    return 0; }
