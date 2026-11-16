#include <stdio.h>
#include <string.h>
#include "../core/wad18.h"
#include "../core/sha256.h"
#include "../breakthrough/r3.h"
#define CYCLES 1000000
static uint32_t xs=2463534242u;
static uint32_t rnd(void){ xs^=xs<<13; xs^=xs>>17; xs^=xs<<5; return xs; }
int main(void){
    int fails=0; char hx[65]; uint8_t dg[32]; sha256_ctx c;
    sha256_init(&c); sha256_update(&c,(const uint8_t*)"abc",3); sha256_final(&c,dg); sha256_hex(dg,hx);
    if(strcmp(hx,"ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad")){puts("FAIL sha256 KAT");return 1;}
    wad_t m=WAD_ONE, k=4*WAD_ONE, dt=WAD_ONE/1000;
    r3_design d=r3_design_damper(m,k,dt);
    printf("R3 design: w=%lld s=%lld a=%lld c=%lld (wad)\n",(long long)d.w,(long long)d.s,(long long)d.a,(long long)d.c);
    printf("CERT C1 feasible  s^2>=4w*1e18 (exact 128b): %s\n",d.ok_feas?"PASS":"FAIL");
    printf("CERT C2 minimal   (s-1)^2<4w*1e18 (exact 128b): %s\n",d.ok_min?"PASS":"FAIL");
    printf("CERT C3 stability 0<a<=1 => det in [0,1): %s\n",d.ok_range?"PASS":"FAIL");
    fails+=!d.ok_feas+!d.ok_min+!d.ok_range;
    { wad_t x=WAD_ONE,v=0,minx=x; int i;
      for(i=0;i<20000;i++){ wad_t a=wad_div(wad_add(wad_mul(wad_sub(0,k),x),wad_mul(wad_sub(0,d.c),v)),m);
        v=wad_add(v,wad_mul(dt,a)); x=wad_add(x,wad_mul(dt,v)); if(x<minx)minx=x; }
      int ok=(minx>=-1000000LL)&&(x<1000000000LL&&x>-1000000000LL);
      printf("CERT C4 free decay 20000 cycles: min x=%lld, no overshoot beyond 1e-12 and final |x|<1e-9: %s\n",(long long)minx,ok?"PASS":"FAIL"); fails+=!ok; }
    sha256_init(&c);
    { wad_t x=0,v=0,maxx=0; int i;
      for(i=0;i<CYCLES;i++){
        wad_t f=((wad_t)(rnd()&0xFFFFu)-32768)*152587890625LL;
        wad_t a=wad_div(wad_add(f,wad_add(wad_mul(wad_sub(0,k),x),wad_mul(wad_sub(0,d.c),v))),m);
        v=wad_add(v,wad_mul(dt,a)); x=wad_add(x,wad_mul(dt,v));
        sha256_put_i64(&c,x); sha256_put_i64(&c,v);
        if((x<0?-x:x)>maxx) maxx=(x<0?-x:x); }
      sha256_final(&c,dg); sha256_hex(dg,hx);
      printf("CERT C5 saturation/overflow flag clear: %s\n",wad_overflow?"FAIL":"PASS"); fails+=wad_overflow;
      printf("cycles=%d final x=%lld v=%lld max|x|=%lld\n",CYCLES,(long long)x,(long long)v,(long long)maxx);
      printf("TRACE_SHA256 %s\n",hx); }
    printf("RESULT %s\n",fails?"FAIL":"ALL CERTIFICATE CHECKS PASS");
    return fails?1:0; }
