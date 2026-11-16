#!/bin/sh
# Mutation gate: every non-equivalent mutant of core/wad128.c must be KILLED by the oracles. Exit 1 if any survives.
set -u; T=$(mktemp -d); fail=0
run(){ # name sed-expr
  cp core/wad128.c "$T/wad128.c"; sed -i "$2" "$T/wad128.c"
  cmp -s core/wad128.c "$T/wad128.c" && { echo "MUTANT $1: NOT APPLIED"; fail=1; return; }
  cc -std=c99 -O2 -w -Icore benchmark/wad128_bigoracle.c core/wad18.c "$T/wad128.c" -o "$T/b" || { fail=1; return; }
  if "$T/b" ff >/dev/null 2>&1; then echo "MUTANT $1: SURVIVED (bad)"; fail=1; else echo "MUTANT $1: killed"; fi; }
run rounding_tie        's|if(u128_cmp(r,t)>=0)|if(u128_cmp(r,t)>0)|'
run negation_carry      's|a.hi=~a.hi+(a.lo==0u?1u:0u);|a.hi=~a.hi;|'
run mul256_c1_p1lo      's|t=p0.hi; s=t+p1.lo; if(s<t)c1++;|t=p0.hi; s=t+p1.lo;|'
run mul256_c1_p2lo      's|t=s; s=t+p2.lo; if(s<t)c1++; o\[1\]=s;|t=s; s=t+p2.lo; o[1]=s;|'
run mul256_c2_plus_c1   's|t=s; s=t+c1; if(s<t)c2++; o\[2\]=s;|t=s; s=t+c1; o[2]=s;|'
run udiv_top_off_by_one 's|for(i=top;i>=0;i--){|for(i=top-1;i>=0;i--){|'
run udiv_qbit_index     's|q\[i/64\]\|=((uint64_t)1u<<(i%64));|q[i/64]\|=((uint64_t)1u<<((i+1)%64));|'
runw(){ # name sed-expr  (mutates core/wad18.c, judged by benchmark/div_equiv.c)
  cp core/wad18.c "$T/wad18.c"; sed -i "$2" "$T/wad18.c"
  cmp -s core/wad18.c "$T/wad18.c" && { echo "MUTANT $1: NOT APPLIED"; fail=1; return; }
  cc -std=c99 -O2 -w -DWAD_TEST_COUNTERS -Icore benchmark/div_equiv.c "$T/wad18.c" -o "$T/d" || { fail=1; return; }
  if "$T/d" ff >/dev/null 2>&1; then echo "MUTANT $1: SURVIVED (bad)"; fail=1; else echo "MUTANT $1: killed"; fi; }
runw div64_tie             's|if(rem>=d-rem){ if(++q.lo==0u) q.hi++; }|if(rem>d-rem){ if(++q.lo==0u) q.hi++; }|'
runw div64_short_skip_top  's|for(i=3;i>=0;i--){ uint64_t cur=(r<<32)\|u\[i\];|for(i=2;i>=0;i--){ uint64_t cur=(r<<32)\|u[i];|'
runw div64_drop_qhat_test  's|while(qhat>=0x100000000ULL\|\|qhat\*vn\[0\]>((rhat<<32)\|un\[j\]))|while(qhat>=0x100000000ULL)|'
runw div64_no_normalize    's|un\[0\]=u\[0\]<<s;|un[0]=u[0];|'
runw div64_remainder_shift 's|rem=(((uint64_t)un\[1\]<<32)\|un\[0\])>>s;|rem=(((uint64_t)un[1]<<32)\|un[0]);|'
runr(){ # name file sed-expr  (mutates core file; judged by benchmark/reentrant_test.c flag/thread cases)
  rm -rf "$T/r"; mkdir -p "$T/r/core" "$T/r/breakthrough" "$T/r/benchmark"; cp core/* "$T/r/core/"; cp breakthrough/* "$T/r/breakthrough/"; cp benchmark/reentrant_test.c "$T/r/benchmark/"
  sed -i "$3" "$T/r/$2"
  cmp -s "$2" "$T/r/$2" && { echo "MUTANT $1: NOT APPLIED"; fail=1; return; }
  cc -std=c99 -O2 -w -pthread "$T/r/benchmark/reentrant_test.c" "$T/r/core/wad18.c" "$T/r/core/wad128.c" "$T/r/breakthrough/r3.c" "$T/r/breakthrough/r3_128.c" -o "$T/r/t" || { fail=1; return; }
  if "$T/r/t" ff >/dev/null 2>&1; then echo "MUTANT $1: SURVIVED (bad)"; fail=1; else echo "MUTANT $1: killed"; fi; }
runr flag_div_zero_not_set   core/wad18.c  's|if(b==0){ \*ovf=1; return a<0?-WMAX:WMAX; }|if(b==0){ return a<0?-WMAX:WMAX; }|'
runr flag_add_overflow_lost  core/wad18.c  's|if(((a^r)\&(b^r))<0){ \*ovf=1;|if(((a^r)\&(b^r))<0){|'
runr flag_finish_lost        core/wad18.c  's|if(q.hi\|\|q.lo>(uint64_t)WMAX){ \*ovf=1;|if(q.hi\|\|q.lo>(uint64_t)WMAX){|'
runr flag_callee_clears      core/wad18.c  's|wad_t wad_add_f(wad_t a,wad_t b,int \*ovf){|wad_t wad_add_f(wad_t a,wad_t b,int *ovf){ *ovf=0;|'
runr flag128_finish_lost     core/wad128.c 's|if(q\[2\]\|\|q\[3\]\|\|(q\[1\]>>63)){ \*ovf=1;|if(q[2]\|\|q[3]\|\|(q[1]>>63)){|'
runr flag128_div_zero_lost   core/wad128.c 's|if(b.hi==0u\&\&b.lo==0u){ \*ovf=1;|if(b.hi==0u\&\&b.lo==0u){|'
# Equivalent (unreachable for |x|<=2^127, argued): mul256 p1.hi+p2.hi carry (both partials < 2^63), udiv shift-carry (remainder < divisor <= 2^127).
rm -rf "$T"; [ $fail = 0 ] && echo "MUTATION GATE PASS" || { echo "MUTATION GATE FAIL"; exit 1; }
