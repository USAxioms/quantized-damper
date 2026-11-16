/* Oracle test: compares core/wad128 against native unsigned __int128 (test-only; the core never uses it). */
#include <stdio.h>
#include "../core/wad128.h"
#ifndef __SIZEOF_INT128__
int main(void){ puts("ORACLE SKIPPED: no native __int128 on this compiler"); return 0; }
#else
__extension__ typedef unsigned __int128 U;
static uint64_t s=0xD1B54A32D192ED03ULL;
static uint64_t r64(void){ s^=s<<13; s^=s>>7; s^=s<<17; return s; }
static U rbits(int bits){ U v=((U)r64()<<64)|r64(); return bits>=128?v:(v&((((U)1)<<bits)-1)); }
static w128 pk(U v,int neg){ w128 r; r.hi=(uint64_t)(v>>64); r.lo=(uint64_t)v;
    if(neg){ r.lo=~r.lo+1u; r.hi=~r.hi+(r.lo==0u?1u:0u); } return r; }
static U up(w128 w){ U v=((U)w.hi<<64)|w.lo; return v; }
static U rdiv(U n,U d){ U q=n/d,r=n%d; if(r>=d-r) q++; return q; }
int main(void){
    const U ONE=(U)1000000000000000000ULL; long mism=0,mulc=0,divc=0,addc=0,i;
    for(i=0;i<300000;i++){
        int la=1+(int)(r64()%100), lb=1+(int)(r64()%(126-la>1?126-la:1)); U a=rbits(la),b=rbits(lb); int na=r64()&1,nb=r64()&1;
        U p=a*b; U q; w128 r; wad128_overflow=0;
        r=w128_mul(pk(a,na),pk(b,nb)); q=rdiv(p,ONE);
        if(q>>126){ if(!wad128_overflow) mism++; }                      /* result too big -> must saturate+flag */
        else { int neg=(na!=nb)&&q!=0; w128 e=pk(q,neg); if(wad128_overflow||r.hi!=e.hi||r.lo!=e.lo) mism++; }
        mulc++;
        { int lb2=1+(int)(r64()%126); int la2=1+(int)(r64()%66); U a2=rbits(la2),b2=rbits(lb2); int n1=r64()&1,n2=r64()&1;
          if(b2!=0){ U n=a2*ONE; U q2=rdiv(n,b2); wad128_overflow=0; r=w128_div(pk(a2,n1),pk(b2,n2));
            if(q2>>126){ if(!wad128_overflow) mism++; }
            else { int neg=(n1!=n2)&&q2!=0; w128 e=pk(q2,neg); if(wad128_overflow||r.hi!=e.hi||r.lo!=e.lo) mism++; }
            divc++; } }
        { U a3=rbits(1+(int)(r64()%120)),b3=rbits(1+(int)(r64()%120)); int n1=r64()&1,n2=r64()&1;
          w128 x=pk(a3,n1),y=pk(b3,n2); wad128_overflow=0; r=w128_add(x,y);
          { U sa=n1?(U)0-a3:a3, sb=n2?(U)0-b3:b3, e=sa+sb; if(wad128_overflow||up(r)!=e) mism++; }
          wad128_overflow=0; r=w128_sub(x,y);
          { U sa=n1?(U)0-a3:a3, sb=n2?(U)0-b3:b3, e=sa-sb; if(wad128_overflow||up(r)!=e) mism++; } addc++; }
    }
    printf("ORACLE vs native __int128: mul=%ld div=%ld addsub=%ld mismatches=%ld %s\n",mulc,divc,addc,mism,mism?"FAIL":"PASS");
    return mism?1:0; }
#endif
