/* Reentrancy checks for the *_f API. Test code only (uses pthreads); the core stays libc-free. */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <pthread.h>
#include "../core/wad128.h"
#include "../breakthrough/r3.h"
#include "../breakthrough/r3_128.h"
#define NT 8
#define ROUNDS 3
static uint64_t s=0x9E3779B97F4A7C15ULL;
static uint64_t r64(void){ s^=s<<13; s^=s>>7; s^=s<<17; return s; }
typedef struct { int id; uint64_t hash; int flag; } job;
static void fnv(uint64_t*h,uint64_t v){ int i; for(i=0;i<8;i++){ *h^=(v>>(8*i))&0xFFu; *h*=1099511628211ULL; } }
static void run_job(job*j){
    const wad_t k=4*WAD_ONE,c=3996000000000000000LL,dt=WAD_ONE/1000; wad_t x=(j->id==5)?9*WAD_ONE:0,v=0; uint64_t rs=88172645463325252ULL+(uint64_t)j->id*0x9E3779B97F4A7C15ULL; int i,f=0;
    uint64_t h=1469598103934665603ULL;
    for(i=0;i<200000;i++){ wad_t fe,a; rs^=rs<<13; rs^=rs>>7; rs^=rs<<17; fe=((wad_t)(rs&0xFFFFu)-32768)*152587890625LL;
        a=wad_div_f(wad_add_f(fe,wad_add_f(wad_sub_f(0,wad_mul_f(k,x,&f),&f),wad_sub_f(0,wad_mul_f(c,v,&f),&f),&f),&f),WAD_ONE,&f);
        v=wad_add_f(v,wad_mul_f(dt,a,&f),&f); x=wad_add_f(x,wad_mul_f(dt,v,&f),&f); fnv(&h,(uint64_t)x); fnv(&h,(uint64_t)v); }
    j->hash=h; j->flag=f; }
static void*worker(void*p){ run_job((job*)p); return 0; }
int main(int argc,char**argv){
    int ff=(argc>1); (void)argv;   /* ff: skip slow checks once a flag unit case fails (mutation gate) */
    long i,t1=0,t2=0,tm=0,iso=0,fu=0; int r,t; job ref[NT],cur[NT]; pthread_t th[NT];
    (void)i; (void)r64;
#ifndef WAD_NO_GLOBALS
    if(!ff){ for(i=0;i<300000;i++){ wad_t a=(wad_t)r64(),b=(wad_t)(r64()>>(r64()%40)); int f; wad_t x,y; w128 A,B,X,Y; int g;
        f=0; wad_overflow=0; x=wad_mul_f(a,b,&f); y=wad_mul(a,b); if(x!=y||(f!=0)!=(wad_overflow!=0)){ t1++; }
        f=0; wad_overflow=0; x=wad_div_f(a,b,&f); y=wad_div(a,b); if(x!=y||(f!=0)!=(wad_overflow!=0)){ t1++; }
        f=0; wad_overflow=0; x=wad_add_f(a,b,&f); y=wad_add(a,b); if(x!=y||(f!=0)!=(wad_overflow!=0)){ t1++; }
        f=0; wad_overflow=0; x=wad_sub_f(a,b,&f); y=wad_sub(a,b); if(x!=y||(f!=0)!=(wad_overflow!=0)){ t1++; }
        A.hi=r64()>>(r64()%64); A.lo=r64(); B.hi=r64()>>(r64()%64); B.lo=(r64()%7==0)?0u:r64(); if(r64()%5==0){ B.hi=0u; }
        g=0; wad128_overflow=0; X=w128_mul_f(A,B,&g); Y=w128_mul(A,B); if(X.hi!=Y.hi||X.lo!=Y.lo||(g!=0)!=(wad128_overflow!=0)){ t2++; }
        g=0; wad128_overflow=0; X=w128_div_f(A,B,&g); Y=w128_div(A,B); if(X.hi!=Y.hi||X.lo!=Y.lo||(g!=0)!=(wad128_overflow!=0)){ t2++; }
        g=0; wad128_overflow=0; X=w128_add_f(A,B,&g); Y=w128_add(A,B); if(X.hi!=Y.hi||X.lo!=Y.lo||(g!=0)!=(wad128_overflow!=0)){ t2++; }
        g=0; wad128_overflow=0; X=w128_sub_f(A,B,&g); Y=w128_sub(A,B); if(X.hi!=Y.hi||X.lo!=Y.lo||(g!=0)!=(wad128_overflow!=0)){ t2++; } }
    for(i=0;i<500;i++){ wad_t m=WAD_ONE/10+(wad_t)(r64()%(uint64_t)(19*(WAD_ONE/10))),k=WAD_ONE/10+(wad_t)(r64()%(uint64_t)(89*(WAD_ONE/10))); int f=0; r3_design a,b;
        a=r3_design_damper_f(m,k,WAD_ONE/1000,&f); wad_overflow=0; b=r3_design_damper(m,k,WAD_ONE/1000);
        if(a.w!=b.w||a.s!=b.s||a.a!=b.a||a.c!=b.c||a.ok_feas!=b.ok_feas||a.ok_min!=b.ok_min||a.ok_range!=b.ok_range||(f!=0)!=(wad_overflow!=0)){ t2++; } } }
#endif
    { /* deterministic flag unit cases (reentrant API): flag must be set exactly when the result saturates */
      int f; w128 big,one128,z,r128; one128=w128_from64(WAD_ONE); z=w128_from64(0); big.hi=0x7FFFFFFFFFFFFFFFULL; big.lo=0xFFFFFFFFFFFFFFFFULL;
      f=0; (void)wad_add_f(INT64_MAX,1,&f); if(f==0){ fu++; }
      f=0; (void)wad_add_f(5*WAD_ONE,4*WAD_ONE,&f); if(f!=0){ fu++; }
      f=0; (void)wad_sub_f(0,INT64_MIN,&f); if(f==0){ fu++; }
      f=0; (void)wad_mul_f(9*WAD_ONE,9*WAD_ONE,&f); if(f==0){ fu++; }
      f=0; (void)wad_mul_f(3*WAD_ONE,2*WAD_ONE,&f); if(f!=0){ fu++; }
      f=0; (void)wad_div_f(WAD_ONE,0,&f); if(f==0){ fu++; }
      f=0; (void)wad_div_f(9*WAD_ONE,WAD_ONE/100,&f); if(f==0){ fu++; }
      f=0; (void)wad_div_f(6*WAD_ONE,3*WAD_ONE,&f); if(f!=0){ fu++; }
      f=0; (void)w128_add_f(big,one128,&f); if(f==0){ fu++; }
      f=0; r128=w128_add_f(one128,one128,&f); if(f!=0||r128.lo!=2*(uint64_t)WAD_ONE){ fu++; }
      f=0; (void)w128_div_f(one128,z,&f); if(f==0){ fu++; }
      f=0; (void)w128_mul_f(big,big,&f); if(f==0){ fu++; }
      f=0; r128=w128_mul_f(one128,one128,&f); if(f!=0||r128.lo!=(uint64_t)WAD_ONE||r128.hi!=0u){ fu++; }
      f=7; (void)wad_add_f(1,1,&f); if(f!=7){ fu++; }     /* callee never clears the flag */
    }
    if(ff&&fu){ printf("REENTRANT TEST FASTFAIL flag_unit_failures=%ld RESULT FAIL\n",fu); return 1; }
    for(t=0;t<NT;t++){ ref[t].id=t; run_job(&ref[t]); }                       /* sequential reference */
    for(r=0;r<ROUNDS;r++){
        for(t=0;t<NT;t++){ cur[t].id=t; cur[t].hash=0; cur[t].flag=-1; pthread_create(&th[t],0,worker,&cur[t]); }
        for(t=0;t<NT;t++){ pthread_join(th[t],0); }
        for(t=0;t<NT;t++){ if(cur[t].hash!=ref[t].hash||cur[t].flag!=ref[t].flag){ tm++; } if((cur[t].flag!=0)!=(t==5)){ iso++; } } }
    printf("REENTRANT TEST wrapper_mismatch64=%ld wrapper_mismatch128_r3=%ld threads=%d rounds=%d thread_mismatch=%ld flag_isolation_violations=%ld flag_unit_failures=%ld RESULT %s\n",
        t1,t2,NT,ROUNDS,tm,iso,fu,(t1||t2||tm||iso||fu)?"FAIL":"PASS");
    return (t1||t2||tm||iso||fu)?1:0; }
