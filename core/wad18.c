#include "wad18.h"
#ifndef WAD_NO_GLOBALS
int wad_overflow = 0;
#endif
#define WMAX INT64_MAX
u128 u128_mul64(uint64_t a, uint64_t b){
    uint64_t a0=a&0xFFFFFFFFu,a1=a>>32,b0=b&0xFFFFFFFFu,b1=b>>32;
    uint64_t p00=a0*b0,p01=a0*b1,p10=a1*b0,p11=a1*b1;
    uint64_t mid=(p00>>32)+(p01&0xFFFFFFFFu)+(p10&0xFFFFFFFFu);
    u128 r; r.lo=(p00&0xFFFFFFFFu)|(mid<<32);
    r.hi=p11+(p01>>32)+(p10>>32)+(mid>>32); return r;
}
int u128_cmp(u128 a,u128 b){
    if(a.hi!=b.hi) { return a.hi<b.hi?-1:1; }
    if(a.lo!=b.lo) { return a.lo<b.lo?-1:1; }
    return 0;
}
#ifdef WAD_TEST_COUNTERS
unsigned long wad_negsub_hits=0;   /* must stay 0: proof below says the trial quotient digit is exact */
#endif
/* 128-by-64 division, quotient rounded half up (magnitudes). Word-wise (32-bit digits) schoolbook division,
 * Knuth Algorithm D / Hacker's Delight divmnu (add-back omitted, see proof below). Bit-identical to the former bitwise loop (see benchmark/div_equiv.c). */
u128 wad_udivround(u128 n,uint64_t d){
    uint32_t u[4],qd[4]={0u,0u,0u,0u}; uint64_t rem; u128 q; int i;
    u[0]=(uint32_t)n.lo; u[1]=(uint32_t)(n.lo>>32); u[2]=(uint32_t)n.hi; u[3]=(uint32_t)(n.hi>>32);
    if((d>>32)==0u){                                   /* one-digit divisor: short division */
        uint64_t r=0u;
        for(i=3;i>=0;i--){ uint64_t cur=(r<<32)|u[i]; qd[i]=(uint32_t)(cur/d); r=cur%d; }
        rem=r;
    } else {                                           /* two-digit divisor */
        uint32_t un[5],vn[2]; uint32_t vh=(uint32_t)(d>>32); int s=0,j; uint64_t dn;
        while((vh&0x80000000u)==0u){ vh<<=1; s++; }
        dn=d<<s; vn[1]=(uint32_t)(dn>>32); vn[0]=(uint32_t)dn;
        un[4]=(s!=0)?(u[3]>>(32-s)):0u;
        for(i=3;i>0;i--) un[i]=(u[i]<<s)|((s!=0)?(u[i-1]>>(32-s)):0u);
        un[0]=u[0]<<s;
        for(j=2;j>=0;j--){
            uint64_t num=((uint64_t)un[j+2]<<32)|un[j+1],qhat=num/vn[1],rhat=num-qhat*vn[1],carry=0u,borrow=0u,sub;
            while(qhat>=0x100000000ULL||qhat*vn[0]>((rhat<<32)|un[j])){ qhat--; rhat+=vn[1]; if(rhat>=0x100000000ULL) break; }
            for(i=0;i<2;i++){ uint64_t p=qhat*vn[i]+carry; carry=p>>32; sub=(uint64_t)un[i+j]-(p&0xFFFFFFFFULL)-borrow; un[i+j]=(uint32_t)sub; borrow=sub>>63; }
            sub=(uint64_t)un[j+2]-carry-borrow; un[j+2]=(uint32_t)sub;
            /* Divisor has exactly two digits, so the loop test above compares ALL three relevant dividend digits:
             * passing it means qhat*V <= U, and qhat never drops below the true digit, hence qhat is exact and this
             * subtraction cannot go negative (Knuth D's add-back step is unreachable for a 2-digit divisor). */
#ifdef WAD_TEST_COUNTERS
            if((sub>>63)!=0u){ wad_negsub_hits++; }
#endif
            qd[j]=(uint32_t)qhat;
        }
        rem=(((uint64_t)un[1]<<32)|un[0])>>s;
    }
    q.lo=((uint64_t)qd[1]<<32)|qd[0]; q.hi=((uint64_t)qd[3]<<32)|qd[2];
    if(rem>=d-rem){ if(++q.lo==0u) q.hi++; }
    return q;
}
static wad_t finish(u128 q,int neg,int *ovf){
    if(q.hi||q.lo>(uint64_t)WMAX){ *ovf=1; return neg?-WMAX:WMAX; }
    return neg?-(wad_t)q.lo:(wad_t)q.lo;
}
static uint64_t mag(wad_t a){ return a<0?(uint64_t)0-(uint64_t)a:(uint64_t)a; }
wad_t wad_add_f(wad_t a,wad_t b,int *ovf){
    uint64_t s=(uint64_t)a+(uint64_t)b; wad_t r=(wad_t)s;
    if(((a^r)&(b^r))<0){ *ovf=1; return a<0?-WMAX:WMAX; } return r;
}
wad_t wad_sub_f(wad_t a,wad_t b,int *ovf){
    if(b==INT64_MIN){ *ovf=1; return WMAX; }
    return wad_add_f(a,-b,ovf);
}
wad_t wad_mul_f(wad_t a,wad_t b,int *ovf){
    u128 p=u128_mul64(mag(a),mag(b));
    return finish(wad_udivround(p,(uint64_t)WAD_ONE),(a<0)!=(b<0),ovf);
}
wad_t wad_div_f(wad_t a,wad_t b,int *ovf){
    if(b==0){ *ovf=1; return a<0?-WMAX:WMAX; }
    u128 n=u128_mul64(mag(a),(uint64_t)WAD_ONE);
    return finish(wad_udivround(n,mag(b)),(a<0)!=(b<0),ovf);
}

#ifndef WAD_NO_GLOBALS
/* Convenience API with a sticky global flag (NOT reentrant: not thread/ISR safe). Define WAD_NO_GLOBALS to remove it. */
wad_t wad_add(wad_t a,wad_t b){ int f=0; wad_t r=wad_add_f(a,b,&f); if(f!=0){ wad_overflow=1; } return r; }
wad_t wad_sub(wad_t a,wad_t b){ int f=0; wad_t r=wad_sub_f(a,b,&f); if(f!=0){ wad_overflow=1; } return r; }
wad_t wad_mul(wad_t a,wad_t b){ int f=0; wad_t r=wad_mul_f(a,b,&f); if(f!=0){ wad_overflow=1; } return r; }
wad_t wad_div(wad_t a,wad_t b){ int f=0; wad_t r=wad_div_f(a,b,&f); if(f!=0){ wad_overflow=1; } return r; }
#endif
