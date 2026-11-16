/* Independent oracle: schoolbook bignum on 32-bit limbs (different representation from core), full-width operands + tie cases. */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../core/wad128.h"
static uint64_t s=0xA0761D6478BD642FULL;
static uint64_t r64(void){ s^=s<<13; s^=s>>7; s^=s<<17; return s; }
#define NL 9
static void bmul(const uint32_t*a,int na,const uint32_t*b,int nb,uint32_t*o){
    int i,j; for(i=0;i<na+nb;i++) o[i]=0;
    for(i=0;i<na;i++){ uint64_t c=0; for(j=0;j<nb;j++){ uint64_t t=(uint64_t)a[i]*b[j]+o[i+j]+c; o[i+j]=(uint32_t)t; c=t>>32; } o[i+nb]=(uint32_t)c; } }
static int bcmp(const uint32_t*a,const uint32_t*b){ int i; for(i=NL-1;i>=0;i--){ if(a[i]!=b[i]) return a[i]<b[i]?-1:1; } return 0; }
static void bsub(uint32_t*a,const uint32_t*b){ int i; uint64_t br=0; for(i=0;i<NL;i++){ uint64_t t=(uint64_t)a[i]-b[i]-br; a[i]=(uint32_t)t; br=(t>>63)&1u; } }
static void bshl1(uint32_t*a,uint32_t bit){ int i; uint32_t c=bit; for(i=0;i<NL;i++){ uint32_t n=a[i]>>31; a[i]=(a[i]<<1)|c; c=n; } }
/* q = round_half_up(n / d); n has 8 limbs, d has 4 limbs; q has 8 limbs */
static void bdivr(const uint32_t*n8,const uint32_t*d4,uint32_t*q){
    uint32_t r[NL]={0},d[NL]={0},r2[NL]; int i,k; for(k=0;k<4;k++) d[k]=d4[k]; for(k=0;k<8;k++) q[k]=0;
    for(i=255;i>=0;i--){ bshl1(r,(n8[i/32]>>(i%32))&1u); if(bcmp(r,d)>=0){ bsub(r,d); q[i/32]|=(1u<<(i%32)); } }
    memcpy(r2,r,sizeof r2); bshl1(r2,0);
    if(bcmp(r2,d)>=0){ for(k=0;k<8;k++){ if(++q[k]!=0u) break; } } }
static w128 join(const uint32_t*l,int neg){ w128 r; r.lo=((uint64_t)l[1]<<32)|l[0]; r.hi=((uint64_t)l[3]<<32)|l[2];
    if(neg){ r.lo=~r.lo+1u; r.hi=~r.hi+(r.lo==0u?1u:0u); } return r; }
static void rmag(uint32_t*l,int bits){ int i; for(i=0;i<4;i++){ int lo=i*32; uint32_t v=(uint32_t)r64();
    if(bits<=lo) v=0; else if(bits<lo+32) v&=(uint32_t)((1ull<<(bits-lo))-1ull); l[i]=v; } }
static long mism=0; static int ff=0;   /* ff: stop at first mismatch (mutation gate) */
static void check(int is_mul,const uint32_t*a,int na,const uint32_t*b,int nb){
    if(ff&&mism){ puts("FASTFAIL"); exit(1); }
    uint32_t one[2]={(uint32_t)(1000000000000000000ULL),(uint32_t)(1000000000000000000ULL>>32)};
    uint32_t n[8],q[8],d[4]; w128 r,e; int neg=(na!=nb); int big,i;
    wad128_overflow=0;
    if(is_mul){ bmul(a,4,b,4,n); d[0]=one[0];d[1]=one[1];d[2]=0;d[3]=0; bdivr(n,d,q);
        r=w128_mul(join(a,na),join(b,nb)); }
    else { uint32_t b4[4]; for(i=0;i<4;i++) b4[i]=b[i]; if(!(b4[0]|b4[1]|b4[2]|b4[3])) return;
        bmul(a,4,one,2,n); n[6]=0;n[7]=0; bdivr(n,b4,q); r=w128_div(join(a,na),join(b,nb)); }
    big=(q[4]|q[5]|q[6]|q[7]|(q[3]>>31))!=0u;
    if(big){ if(!wad128_overflow) mism++; return; }
    { int zero=!(q[0]|q[1]|q[2]|q[3]); e=join(q,neg&&!zero); if(wad128_overflow||e.hi!=r.hi||e.lo!=r.lo) mism++; } }
int main(int argc,char**argv){
    long i,cnt=0,ties=0; uint32_t a[4],b[4];
    ff=(argc>1); (void)argv;
    for(i=0;i<60000;i++){ int la=1+(int)(r64()%127),lb=1+(int)(r64()%127),na=r64()&1,nb=r64()&1;
        rmag(a,la); rmag(b,lb); check(1,a,na,b,nb); check(0,a,na,b,nb); cnt+=2; }
    /* ties: mul: a=0.5 (5e17 units), b odd => result exactly .5 unit.  div: b=2.0 (2e18 units), a odd => a/2 */
    for(i=0;i<20000;i++){ uint32_t half[4]={(uint32_t)500000000000000000ULL,(uint32_t)(500000000000000000ULL>>32),0,0};
        uint32_t two[4]={(uint32_t)2000000000000000000ULL,(uint32_t)(2000000000000000000ULL>>32),0,0};
        int na=r64()&1,nb=r64()&1; rmag(b,1+(int)(r64()%100)); b[0]|=1u; rmag(a,1+(int)(r64()%66)); a[0]|=1u;
        check(1,half,na,b,nb); check(0,a,na,two,nb); ties+=2; }
    /* targeted: reaches the mul256 "+c1" carry that random fuzzing hits with probability ~2^-63.
       a=(hi=2^32-1, lo=2^32-1), b=(hi=2^32+1, lo=2^32-1): p3.lo=2^64-1, p1.hi=p2.hi=0, c1>=1 */
    { uint32_t ta[4]={0xFFFFFFFFu,0u,0xFFFFFFFFu,0u}, tb[4]={0xFFFFFFFFu,0u,1u,1u}; int sa,sb;
      for(sa=0;sa<2;sa++) for(sb=0;sb<2;sb++){ check(1,ta,sa,tb,sb); check(1,tb,sa,ta,sb); cnt+=2; } }
    printf("BIGORACLE (32-bit-limb bignum): random=%ld ties=%ld mismatches=%ld %s\n",cnt,ties,mism,mism?"FAIL":"PASS");
    return mism?1:0; }
