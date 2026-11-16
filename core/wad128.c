#include "wad128.h"
#ifndef WAD_NO_GLOBALS
int wad128_overflow=0;
#endif
static int isneg(w128 a){ return (int)(a.hi>>63); }
static w128 wneg(w128 a){ a.lo=~a.lo+1u; a.hi=~a.hi+(a.lo==0u?1u:0u); return a; }
static w128 wmax(int neg){ w128 r; r.hi=0x7FFFFFFFFFFFFFFFULL; r.lo=0xFFFFFFFFFFFFFFFFULL; return neg?wneg(r):r; }
static u128 mag(w128 a){ return isneg(a)?wneg(a):a; }
w128 w128_from64(wad_t a){ w128 r; r.lo=(uint64_t)a; r.hi=a<0?0xFFFFFFFFFFFFFFFFULL:0u; return r; }
int w128_to64(w128 a, wad_t *out){
    int n=isneg(a);
    if(n){ if(a.hi!=0xFFFFFFFFFFFFFFFFULL || (a.lo>>63)==0u) return 0; }
    else { if(a.hi!=0u || (a.lo>>63)!=0u) return 0; }
    *out=(wad_t)a.lo; return 1; }
int w128_cmp(w128 a, w128 b){
    int na=isneg(a), nb=isneg(b);
    if(na!=nb) return na?-1:1;
    if(a.hi!=b.hi) return a.hi<b.hi?-1:1;
    if(a.lo!=b.lo) return a.lo<b.lo?-1:1;
    return 0; }
w128 w128_add_f(w128 a, w128 b, int *ovf){
    w128 r; r.lo=a.lo+b.lo; r.hi=a.hi+b.hi+(r.lo<a.lo?1u:0u);
    if(isneg(a)==isneg(b) && isneg(r)!=isneg(a)){ *ovf=1; return wmax(isneg(a)); }
    return r; }
w128 w128_sub_f(w128 a, w128 b, int *ovf){
    if(b.hi==0x8000000000000000ULL && b.lo==0u){ *ovf=1; return wmax(0); }
    return w128_add_f(a,wneg(b),ovf); }
static void mul256(u128 a,u128 b,uint64_t o[4]){
    u128 p0=u128_mul64(a.lo,b.lo),p1=u128_mul64(a.lo,b.hi),p2=u128_mul64(a.hi,b.lo),p3=u128_mul64(a.hi,b.hi);
    uint64_t c1=0,c2=0,t,s;
    o[0]=p0.lo;
    t=p0.hi; s=t+p1.lo; if(s<t)c1++; t=s; s=t+p2.lo; if(s<t)c1++; o[1]=s;
    t=p1.hi; s=t+p2.hi; if(s<t)c2++; t=s; s=t+p3.lo; if(s<t)c2++; t=s; s=t+c1; if(s<t)c2++; o[2]=s;
    o[3]=p3.hi+c2; }
/* 256-bit / 128-bit bitwise long division, quotient rounded half up */
static void udiv(const uint64_t n[4],u128 d,uint64_t q[4]){
    u128 r={0,0}; int i,k,top=-1; uint64_t carry,bit;
    for(k=0;k<4;k++) q[k]=0;
    for(k=3;k>=0&&top<0;k--){ if(n[k]){ int b=63; while(!((n[k]>>b)&1u)) b--; top=k*64+b; } }
    for(i=top;i>=0;i--){                      /* leading zero bits of n cannot change r or q: skipped */
        bit=(n[i/64]>>(i%64))&1u; carry=r.hi>>63;
        r.hi=(r.hi<<1)|(r.lo>>63); r.lo=(r.lo<<1)|bit;
        if(carry||u128_cmp(r,d)>=0){
            uint64_t bw=r.lo<d.lo?1u:0u; r.lo-=d.lo; r.hi=r.hi-d.hi-bw; q[i/64]|=((uint64_t)1u<<(i%64)); } }
    { u128 t; uint64_t bw=d.lo<r.lo?1u:0u; t.lo=d.lo-r.lo; t.hi=d.hi-r.hi-bw;   /* t=d-r */
      if(u128_cmp(r,t)>=0){ for(k=0;k<4;k++){ if(++q[k]!=0u) break; } } } }
static w128 finish(const uint64_t q[4],int neg,int *ovf){
    w128 v;
    if(q[2]||q[3]||(q[1]>>63)){ *ovf=1; return wmax(neg); }
    v.hi=q[1]; v.lo=q[0]; return neg?wneg(v):v; }
w128 w128_mul_f(w128 a,w128 b,int *ovf){
    uint64_t n[4],q[4]; u128 one; one.hi=0; one.lo=(uint64_t)WAD_ONE;
    mul256(mag(a),mag(b),n); udiv(n,one,q); return finish(q,isneg(a)!=isneg(b),ovf); }
w128 w128_div_f(w128 a,w128 b,int *ovf){
    uint64_t n[4],q[4]; u128 one,db=mag(b);
    if(b.hi==0u&&b.lo==0u){ *ovf=1; return wmax(isneg(a)); }
    one.hi=0; one.lo=(uint64_t)WAD_ONE;
    mul256(mag(a),one,n); udiv(n,db,q); return finish(q,isneg(a)!=isneg(b),ovf); }

#ifndef WAD_NO_GLOBALS
w128 w128_add(w128 a,w128 b){ int f=0; w128 r=w128_add_f(a,b,&f); if(f!=0){ wad128_overflow=1; } return r; }
w128 w128_sub(w128 a,w128 b){ int f=0; w128 r=w128_sub_f(a,b,&f); if(f!=0){ wad128_overflow=1; } return r; }
w128 w128_mul(w128 a,w128 b){ int f=0; w128 r=w128_mul_f(a,b,&f); if(f!=0){ wad128_overflow=1; } return r; }
w128 w128_div(w128 a,w128 b){ int f=0; w128 r=w128_div_f(a,b,&f); if(f!=0){ wad128_overflow=1; } return r; }
#endif
