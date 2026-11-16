/* Equivalence of core wad_udivround (word-wise) against the former bitwise loop (reference, kept here).
 * Build with -DWAD_TEST_COUNTERS to count negative-subtract events (must be 0). Modes: (none)=gate, "search"=extended biased fuzz (200M). */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../core/wad18.h"
#ifdef WAD_TEST_COUNTERS
extern unsigned long wad_negsub_hits;
#endif
static u128 ref_div(u128 n,uint64_t d){
    u128 q={0,0}; uint64_t rem=0; int i;
    for(i=127;i>=0;i--){
        uint64_t bit=(i>=64)?((n.hi>>(i-64))&1u):((n.lo>>i)&1u);
        uint64_t carry=rem>>63; rem=(rem<<1)|bit;
        q.hi=(q.hi<<1)|(q.lo>>63); q.lo<<=1;
        if(carry||rem>=d){ rem-=d; q.lo|=1u; } }
    if(rem>=d-rem){ if(++q.lo==0) q.hi++; }
    return q; }
static uint64_t s=0x2545F4914F6CDD1DULL;
static uint64_t r64(void){ s^=s<<13; s^=s>>7; s^=s<<17; return s; }
static uint64_t rbits64(int b){ uint64_t v=r64(); return b>=64?v:(v&((1ULL<<b)-1ULL)); }
static unsigned long mism=0,total=0,found=0; static int search=0,ff=0;   /* ff: stop at first mismatch (mutation gate) */
static void chk(u128 n,uint64_t d){
    u128 a,b;
#ifdef WAD_TEST_COUNTERS
    unsigned long h0=wad_negsub_hits;
#endif
    if(d==0u){ return; }
    if(ff&&mism){ printf("FASTFAIL\n"); exit(1); }
    a=wad_udivround(n,d); b=ref_div(n,d); total++;
    if(a.hi!=b.hi||a.lo!=b.lo){ mism++; if(mism<4) printf("MISMATCH n=%016llx%016llx d=%016llx\n",(unsigned long long)n.hi,(unsigned long long)n.lo,(unsigned long long)d); }
#ifdef WAD_TEST_COUNTERS
    if(wad_negsub_hits>h0&&found<12){ found++; printf("NEGATIVE-SUBTRACT (proof violated) n.hi=0x%016llxULL n.lo=0x%016llxULL d=0x%016llxULL\n",(unsigned long long)n.hi,(unsigned long long)n.lo,(unsigned long long)d); }
#endif
}
static u128 add64(u128 x,uint64_t y){ x.lo+=y; if(x.lo<y) x.hi++; return x; }
static u128 sub64(u128 x,uint64_t y){ if(x.lo<y) x.hi--; x.lo-=y; return x; }
int main(int argc,char**argv){
    long i; uint64_t sp[]={1ULL,2ULL,3ULL,0xFFFFFFFFULL,0x100000000ULL,0x100000001ULL,0x7FFFFFFFFFFFFFFFULL,0x8000000000000000ULL,
        0x8000000100000000ULL,0xFFFFFFFF00000001ULL,0xFFFFFFFFFFFFFFFFULL,1000000000000000000ULL,3814697265625ULL,262144ULL};
    int ns=(int)(sizeof sp/sizeof sp[0]),a,b;
    search=(argc>1&&strcmp(argv[1],"search")==0); (void)found; ff=(argc>1&&strcmp(argv[1],"ff")==0);
    if(!search){
        for(i=0;i<3000000;i++){ u128 n; n.hi=rbits64(1+(int)(r64()%64)); n.lo=rbits64(1+(int)(r64()%64)); if(r64()&1u) n.hi=0u;
            chk(n,rbits64(1+(int)(r64()%64))|1u); chk(n,rbits64(1+(int)(r64()%64))); }
        for(a=0;a<ns;a++) for(b=0;b<ns;b++){ u128 base=u128_mul64(sp[a],sp[b]),m; int k;
            for(k=-2;k<=2;k++){ m=(k<0)?sub64(base,(uint64_t)(-k)):add64(base,(uint64_t)k); chk(m,sp[a]); chk(m,sp[b]); }
            m=add64(base,sp[a]/2u); chk(m,sp[a]); m=add64(base,sp[b]/2u); chk(m,sp[b]); }
        { u128 mx={0xFFFFFFFFFFFFFFFFULL,0xFFFFFFFFFFFFFFFFULL},z={0,0},one={0,1}; for(a=0;a<ns;a++){ chk(mx,sp[a]); chk(z,sp[a]); chk(one,sp[a]); } }
        for(i=0;i<2000000;i++){ uint64_t d=rbits64(1+(int)(r64()%64)); u128 n=u128_mul64(rbits64(64),d); if(d==0u) continue; n=add64(n,r64()%d); chk(n,d); }
    }
    /* biased generator aimed at the add-back correction: normalized-ish top digit, high low digit, n near multiples */
    { long iters=search?200000000L:4000000L;
      for(i=0;i<iters;i++){ uint64_t vh=(r64()&1u)?(0x80000000ULL|(r64()&0xFu)):(0xFFFFFFF0ULL|(r64()&0xFu));
        uint64_t vl=(r64()&1u)?(0xFFFFFFF0ULL|(r64()&0xFu)):(r64()&0xFFFFFFFFULL); uint64_t d=(vh<<32)|vl; u128 n;
        n.hi=(r64()&1u)?(vh|((r64()&1u)?(r64()&0xFFFFFFFF00000000ULL):0u)):rbits64(64); n.lo=(r64()&1u)?0xFFFFFFFFFFFFFFFFULL:r64();
        if(r64()&1u){ n=u128_mul64(r64()&0xFFFFFFFFULL?r64():0xFFFFFFFFFFFFFFFFULL,d); n=add64(n,(r64()&1u)?(d-1u):(r64()%d)); }
        chk(n,d); } }
#ifdef WAD_TEST_COUNTERS
    printf("DIVEQUIV cases=%lu mismatches=%lu negsub_hits=%lu %s\n",total,mism,wad_negsub_hits,(mism||wad_negsub_hits)?"FAIL":"PASS");
#else
    printf("DIVEQUIV cases=%lu mismatches=%lu %s\n",total,mism,mism?"FAIL":"PASS");
#endif
    return (mism
#ifdef WAD_TEST_COUNTERS
        ||wad_negsub_hits
#endif
        )?1:0; }
