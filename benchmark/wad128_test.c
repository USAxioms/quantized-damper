#include <stdio.h>
#include "../core/wad128.h"
#include "../core/sha256.h"
#include "../breakthrough/r3_128.h"
static uint64_t s=0x9E3779B97F4A7C15ULL;
static uint64_t r64(void){ s^=s<<13; s^=s>>7; s^=s<<17; return s; }
static wad_t rv(void){ int sh=(int)(r64()%62); wad_t v=(wad_t)(r64()>>(2+sh)); return (r64()&1u)?-v:v; }
static void put128(sha256_ctx*h,w128 a){ sha256_put_i64(h,(int64_t)a.hi); sha256_put_i64(h,(int64_t)a.lo); }
int main(void){
    int fails=0,i,cmp=0,skipped=0; wad_t o;
    /* T1: differential vs 64-bit core wherever the 64-bit result is in range */
    for(i=0;i<200000;i++){ wad_t a=rv(),b=rv(); wad_t r64v,r128v; w128 q;
        wad_overflow=0; r64v=wad_mul(a,b);
        if(!wad_overflow){ q=w128_mul(w128_from64(a),w128_from64(b)); if(!w128_to64(q,&r128v)||r128v!=r64v){fails++;} cmp++; } else skipped++;
        if(b!=0){ wad_overflow=0; r64v=wad_div(a,b);
            if(!wad_overflow){ q=w128_div(w128_from64(a),w128_from64(b)); if(!w128_to64(q,&r128v)||r128v!=r64v){fails++;} cmp++; } else skipped++; }
        wad_overflow=0; r64v=wad_add(a,b);
        if(!wad_overflow){ q=w128_add(w128_from64(a),w128_from64(b)); if(!w128_to64(q,&r128v)||r128v!=r64v){fails++;} cmp++; }
        wad_overflow=0; r64v=wad_sub(a,b);
        if(!wad_overflow){ q=w128_sub(w128_from64(a),w128_from64(b)); if(!w128_to64(q,&r128v)||r128v!=r64v){fails++;} cmp++; } }
    printf("T1 differential vs wad64: compared=%d skipped_64bit_overflow=%d mismatches=%d %s\n",cmp,skipped,fails,fails?"FAIL":"PASS");
    /* T2: beyond-int64 exact values */
    { w128 a=w128_from64(3*WAD_ONE),b=w128_from64(4*WAD_ONE),p,q; int ok;
      wad128_overflow=0; p=w128_mul(a,b);                       /* 12.0, > int64 max */
      ok=(p.hi==0u&&p.lo==12000000000000000000ULL)&&!w128_to64(p,&o);
      q=w128_div(p,b); ok=ok&&w128_to64(q,&o)&&o==3*WAD_ONE;
      p=w128_mul(w128_mul(p,p),w128_from64(WAD_ONE));            /* 144.0 = 1.44e20: hi=7, lo=14872791484033138688 */
      ok=ok&&p.hi==7u&&p.lo==14872791484033138688ULL;
      ok=ok&&!wad128_overflow; printf("T2 beyond-int64 exact (12.0, 12/4=3): %s\n",ok?"PASS":"FAIL"); fails+=!ok; }
    /* T3: stiff plant impossible in wad64: m=1,k=400 => c~40; free decay + disturbed trace */
    { w128 m=w128_from64(WAD_ONE),dt=w128_from64(WAD_ONE/1000),five=w128_from64(5*WAD_ONE);
      w128 k=w128_mul(w128_mul(five,w128_from64(8*WAD_ONE)),w128_add(five,five)); /* 5*8*(5+5)=400 */
      r3_design128 d=r3_design_damper128(m,k,dt); int ok=d.ok_domain&&d.ok_feas&&d.ok_min&&d.ok_range;
      wad_t c64; int fits=w128_to64(d.c,&c64);
      printf("T3 R3-128 cert: domain=%d feas=%d min=%d range=%d; c fits int64=%d (must be 0): %s\n",d.ok_domain,d.ok_feas,d.ok_min,d.ok_range,fits,(ok&&!fits)?"PASS":"FAIL");
      fails+=!(ok&&!fits);
      { w128 x=w128_from64(WAD_ONE),v=w128_from64(0),mn=x; sha256_ctx h; uint8_t dg[32]; char hx[65]; int j;
        w128 negk=w128_sub(w128_from64(0),k),negc=w128_sub(w128_from64(0),d.c);
        wad128_overflow=0;
        for(j=0;j<20000;j++){ w128 a=w128_div(w128_add(w128_mul(negk,x),w128_mul(negc,v)),m);
            v=w128_add(v,w128_mul(dt,a)); x=w128_add(x,w128_mul(dt,v)); if(w128_cmp(x,mn)<0) mn=x; }
        { wad_t mn64,x64; int ok2=w128_to64(mn,&mn64)&&mn64>=-1000000LL&&w128_to64(x,&x64)&&x64<1000000000LL&&x64>-1000000000LL;
          printf("T3 free decay k=400: min_x=%lld %s\n",(long long)mn64,ok2?"PASS":"FAIL"); fails+=!ok2; }
        sha256_init(&h); x=w128_from64(0); v=w128_from64(0);
        for(j=0;j<200000;j++){ wad_t f=((wad_t)(r64()&0xFFFFu)-32768)*152587890625LL; w128 a;
            a=w128_div(w128_add(w128_from64(f),w128_add(w128_mul(negk,x),w128_mul(negc,v))),m);
            v=w128_add(v,w128_mul(dt,a)); x=w128_add(x,w128_mul(dt,v)); put128(&h,x); put128(&h,v); }
        sha256_final(&h,dg); sha256_hex(dg,hx);
        printf("T3 overflow=%s\n",wad128_overflow?"FAIL":"PASS"); fails+=wad128_overflow;
        printf("WAD128_TRACE_SHA256 %s\n",hx); } }
    printf("RESULT %s\n",fails?"FAIL":"PASS"); return fails?1:0; }
