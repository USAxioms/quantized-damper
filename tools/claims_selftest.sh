#!/bin/sh
# The claim checker must reject broken registers. Each case edits a copy of claims.tsv and must make check_claims.sh fail.
export LC_ALL=C CLAIMS_SELFTEST=1; W=$(mktemp -d); fail=0; T=$(printf '\t')
run(){ # name expected(pass|fail) sed-expression
  rm -rf "$W/c"; mkdir "$W/c"; cp -r build tools sweep_data "$W/c"/ 2>/dev/null; cp claims.tsv "$W/c/claims.tsv"
  [ -n "$3" ] && (cd "$W/c" && sed -i "$3" claims.tsv)
  [ "$2" = fail ] && cmp -s claims.tsv "$W/c/claims.tsv" && { echo "CLAIMSTEST $1: INEFFECTIVE WRONG"; fail=1; return; }
  (cd "$W/c" && sh tools/check_claims.sh >/dev/null 2>&1); rc=$?
  if { [ "$2" = pass ] && [ $rc = 0 ]; } || { [ "$2" = fail ] && [ $rc != 0 ]; }; then echo "CLAIMSTEST $1: ok (expected $2)"; else echo "CLAIMSTEST $1: WRONG (rc=$rc expected $2)"; fail=1; fi; }
run C0_untouched               pass ""
run C1_dep_on_unverified       fail "s/^E1${T}empirical${T}verified${T}reproducible-run${T}A1,A2,A3,A8/E1${T}empirical${T}verified${T}reproducible-run${T}A1,U1/"
run C2_undefined_dependency    fail "s/^E4${T}empirical${T}verified${T}sampled${T}A1,A2,A8/E4${T}empirical${T}verified${T}sampled${T}A1,ZZZ/"
run C3_failing_evidence        fail "s/mismatches=0 PASS' build\/bigoracle.log/mismatches=7 PASS' build\/bigoracle.log/"
run C4_verified_without_cmd    fail "s/^E14${T}empirical${T}verified${T}reproducible-run${T}A2,A5${T}\([^${T}]*\)${T}.*/E14${T}empirical${T}verified${T}reproducible-run${T}A2,A5${T}\1${T}/"
run C5_cycle                   fail "s/^M1${T}derivation${T}verified${T}sampled${T}A1,A7/M1${T}derivation${T}verified${T}sampled${T}A1,M2/"
run C6_refuted_not_reproduced  fail "s/nm build\/emb_wad18.o | grep -qE ' \[bB\] wad_overflow'/nm build\/emb_wad18.o | grep -qE ' [bB] no_such_symbol'/"
rm -rf "$W"; [ $fail = 0 ] && echo "CLAIMS SELFTEST PASS" || { echo "CLAIMS SELFTEST FAIL"; exit 1; }
