/* Machine checks backing the mathematical claims (M1-M4). Samples; does not prove. */
#include <stdio.h>
#include <math.h>
#include "../core/wad18.h"
#include "../breakthrough/r3.h"
static uint64_t s=0x9E3779B97F4A7C15ULL;
static uint64_t r64(void){ s^=s<<13; s^=s>>7; s^=s<<17; return s; }
int main(void){
    long i,idfail=0,certfail=0,devfail=0,rangefail=0,plants=0; long double maxdev=0.0L,maxratio=0.0L;
    const wad_t dts[3]={WAD_ONE/1000,976562500000000LL,WAD_ONE/2000};
    /* M1 spot check: (2-w-a)^2 - 4(1-a) == (a+w)^2 - 4w as a polynomial identity, tested in Z/2^64 */
    for(i=0;i<2000000;i++){ uint64_t a=r64(),w=r64(); uint64_t t=2u-w-a,lhs=t*t-4u*(1u-a),sum=a+w,rhs=sum*sum-4u*w; if(lhs!=rhs){ idfail++; } }
    for(i=0;i<20000;i++){
        wad_t k=(wad_t)(WAD_ONE/10+(wad_t)(r64()%(uint64_t)(89*(WAD_ONE/10)))), m=(wad_t)(WAD_ONE/10+(wad_t)(r64()%(uint64_t)(19*(WAD_ONE/10))));
        wad_t dt=dts[r64()%3]; r3_design d; long double kl,ml,dtl,cf,dev,wl,tol,ratio;
        wad_overflow=0; d=r3_design_damper(m,k,dt);
        if(wad_overflow){ continue; }              /* out of WAD-18 range: skipped, not counted */
        plants++;
        if(!(d.ok_feas&&d.ok_min&&d.ok_range)){ certfail++; }
        if(!(d.s>0&&d.s<=WAD_ONE)){ rangefail++; }  /* M4: s in (0,1] => repeated eigenvalue 1-s/2 in [0.5,1) */
        kl=(long double)k/1e18L; ml=(long double)m/1e18L; dtl=(long double)dt/1e18L;
        cf=2.0L*sqrtl(kl*ml)-kl*dtl; dev=fabsl((long double)d.c/1e18L-cf); if(dev>maxdev){ maxdev=dev; }
        /* Derived quantization bound. u=1e-18. w=dt^2*(k/m) is built from a ROUNDED dt^2 (error <=u/2, amplified by k/m) and a
           rounded product (<=u/2): eps_w <= (u/2)(1+k/m). s=ceil(2 sqrt(w_q)) adds <=u and carries eps_w/sqrt(w).
           c=(a*m)/dt with a=s-w: |c-c_exact| <= (m/dt)(eps_w/sqrt(w) + u + eps_w) + u/dt + u/2, then safety factor 2. */
        wl=dtl*dtl*kl/ml; { long double ew=0.5e-18L*(1.0L+kl/ml);
        tol=2.0L*((ml/dtl)*(ew/sqrtl(wl)+1e-18L+ew)+1e-18L/dtl+0.5e-18L); }
        ratio=dev/tol; if(ratio>maxratio){ maxratio=ratio; } if(dev>tol){ devfail++; } }
    printf("CLAIMS_MATH identity_fail=%ld plants=%ld cert_failures=%ld range_failures=%ld closed_form_outside_derived_bound=%ld RESULT %s\n",
        idfail,plants,certfail,rangefail,devfail,(idfail||certfail||rangefail||devfail||plants<10000)?"FAIL":"PASS");
    printf("REPORT max_dev_vs_closed_form=%.3Le max_dev_over_bound=%.3Le (host libm; not hashed)\n",maxdev,maxratio);
    return (idfail||certfail||rangefail||devfail||plants<10000)?1:0; }
