#include <stdio.h>
#include <string.h>
#include "../core/wad18.h"
#include "../core/sha256.h"
#include "../breakthrough/r3.h"
#define CYCLES 1000000
#define NAX 3
static uint32_t xs=88172645u;
static uint32_t rnd(void){ xs^=xs<<13; xs^=xs>>17; xs^=xs<<5; return xs; }
int main(void){
    const wad_t M[NAX]={WAD_ONE,2*WAD_ONE,WAD_ONE/2}, K[NAX]={4*WAD_ONE,9*WAD_ONE,WAD_ONE};
    wad_t dt=WAD_ONE/1000, C[NAX], x[NAX]={0,0,0}, v[NAX]={0,0,0}; int ax,i,fails=0; r3_design d[NAX];
    sha256_ctx h; uint8_t dg[32]; char hx[65];
    for(ax=0;ax<NAX;ax++){ d[ax]=r3_design_damper(M[ax],K[ax],dt); C[ax]=d[ax].c;
        int ok=d[ax].ok_feas&&d[ax].ok_min&&d[ax].ok_range; fails+=!ok;
        printf("axis%d m=%lld k=%lld c=%lld cert=%s\n",ax,(long long)M[ax],(long long)K[ax],(long long)C[ax],ok?"PASS":"FAIL"); }
    for(ax=0;ax<NAX;ax++){ wad_t px=WAD_ONE,pv=0,mn=px; int j;
        for(j=0;j<20000;j++){ wad_t a=wad_div(wad_add(wad_mul(wad_sub(0,K[ax]),px),wad_mul(wad_sub(0,C[ax]),pv)),M[ax]);
            pv=wad_add(pv,wad_mul(dt,a)); px=wad_add(px,wad_mul(dt,pv)); if(px<mn){mn=px;} }
        int ok=(mn>=-1000000LL)&&px<1000000000LL&&px>-1000000000LL; fails+=!ok;
        printf("axis%d free-decay min_x=%lld final_x=%lld %s\n",ax,(long long)mn,(long long)px,ok?"PASS":"FAIL"); }
    sha256_init(&h);
    for(i=0;i<CYCLES;i++) for(ax=0;ax<NAX;ax++){
        wad_t f=((wad_t)(rnd()&0xFFFFu)-32768)*152587890625LL;
        wad_t a=wad_div(wad_add(f,wad_add(wad_mul(wad_sub(0,K[ax]),x[ax]),wad_mul(wad_sub(0,C[ax]),v[ax]))),M[ax]);
        v[ax]=wad_add(v[ax],wad_mul(dt,a)); x[ax]=wad_add(x[ax],wad_mul(dt,v[ax]));
        sha256_put_i64(&h,x[ax]); sha256_put_i64(&h,v[ax]); }
    sha256_final(&h,dg); sha256_hex(dg,hx);
    printf("overflow=%s\n",wad_overflow?"FAIL":"PASS"); fails+=wad_overflow;
    printf("MULTI_TRACE_SHA256 %s\nRESULT %s\n",hx,fails?"FAIL":"PASS");
    return fails?1:0; }
