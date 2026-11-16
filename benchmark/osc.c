/* Accumulating-error regime: undamped oscillator, semi-implicit Euler, dt=2^-10 (exact in WAD, double, quad).
 * Compares WAD-18 and double against a binary128 reference (test-only, __float128). Report-only. */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "../core/wad18.h"
__extension__ typedef __float128 Q;
#define N 2000000
int main(int argc,char**argv){
    int sh=argc>1?atoi(argv[1]):0; /* amplitude = 2^sh, exact in all three types (-18<=sh<=2) */
    const wad_t dtw=976562500000000LL, kw=4*WAD_ONE; /* 2^-10 * 1e18 exactly */
    wad_t wx=(sh>=0)?(WAD_ONE*((wad_t)1<<sh)):(WAD_ONE>>(-sh)),wv=0; double dx=ldexp(1.0,sh),dv=0.0; Q qx=(Q)ldexp(1.0,sh),qv=0; const double ddt=0.0009765625; const Q qdt=(Q)0.0009765625, ONEQ=(Q)1000000000000000000LL;
    double maxw=0,maxd=0,maxew=0,maxed=0; int i; wad_overflow=0;
    if(argc<=2) printf("cycle,err_x_wad,err_x_double,err_E_wad,err_E_double\n");
    for(i=1;i<=N;i++){
        wad_t a=wad_sub(0,wad_mul(kw,wx)); wv=wad_add(wv,wad_mul(dtw,a)); wx=wad_add(wx,wad_mul(dtw,wv));
        { double da=-4.0*dx; dv=dv+ddt*da; dx=dx+ddt*dv; }
        { Q qa=-4*qx; qv=qv+qdt*qa; qx=qx+qdt*qv; }
        if(i%250000==0||i==N){
            Q xw=(Q)wx/ONEQ, vw=(Q)wv/ONEQ, xd=(Q)dx, vd=(Q)dv;
            Q Eq=(qv*qv+4*qx*qx)/2, Ew=(vw*vw+4*xw*xw)/2, Ed=(vd*vd+4*xd*xd)/2;
            double ew=(double)(xw-qx), ed=(double)(xd-qx), ewe=(double)(Ew-Eq), ede=(double)(Ed-Eq);
            if(fabs(ew)>maxw){maxw=fabs(ew);}
            if(fabs(ed)>maxd){maxd=fabs(ed);}
            if(fabs(ewe)>maxew){maxew=fabs(ewe);}
            if(fabs(ede)>maxed){maxed=fabs(ede);}
            if(argc<=2) printf("%d,%.3e,%.3e,%.3e,%.3e\n",i,ew,ed,ewe,ede); } }
    printf("SUMMARY amp=2^%d cycles=%d max_err_x: wad=%.3e double=%.3e | max_err_E: wad=%.3e double=%.3e | wad_overflow=%d\n",sh,N,maxw,maxd,maxew,maxed,wad_overflow);
    printf("RATIO double/wad: x=%.2f E=%.2f\n",maxd/maxw,maxed/maxew);
    return 0; }
