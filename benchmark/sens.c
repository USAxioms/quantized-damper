#include <stdio.h>
#include <stdlib.h>
#include "../core/wad18.h"
#include "../core/sha256.h"
#include "../breakthrough/r3.h"
int main(int argc,char**argv){
    if(argc<4) return 2;
    wad_t k=atoll(argv[2]), cf=atoll(argv[3]), m=WAD_ONE, dt=WAD_ONE/1000;
    r3_design d=r3_design_damper(m,k,dt); wad_t c=wad_mul(d.c,cf);
    wad_t x=WAD_ONE,v=0,minx=x,maxx=x; sha256_ctx h; uint8_t dg[32]; char hx[65]; int i,cross=0,settle=-1;
    wad_overflow=0; sha256_init(&h);
    for(i=0;i<20000;i++){
        wad_t a=wad_div(wad_add(wad_mul(wad_sub(0,k),x),wad_mul(wad_sub(0,c),v)),m);
        wad_t px=x; v=wad_add(v,wad_mul(dt,a)); x=wad_add(x,wad_mul(dt,v));
        sha256_put_i64(&h,x); sha256_put_i64(&h,v);
        if(x<minx){minx=x;}
        if(x>maxx){maxx=x;}
        if((px>0&&x<0)||(px<0&&x>0)){cross++;}
        if(settle<0&&(x<1000000000000000LL&&x>-1000000000000000LL)) settle=i; }
    sha256_final(&h,dg); sha256_hex(dg,hx);
    printf("{\"tag\":\"%s\",\"k_wad\":%lld,\"c_factor_wad\":%lld,\"c_wad\":%lld,\"r3_ok\":[%d,%d,%d],\"min_x\":%lld,\"max_x\":%lld,\"final_x\":%lld,\"final_v\":%lld,\"zero_crossings\":%d,\"first_cycle_abs_x_lt_1e-3\":%d,\"overflow\":%d,\"trace_sha256\":\"%s\"}\n",
      argv[1],(long long)k,(long long)cf,(long long)c,d.ok_feas,d.ok_min,d.ok_range,(long long)minx,(long long)maxx,(long long)x,(long long)v,cross,settle,wad_overflow,hx);
    return 0; }
