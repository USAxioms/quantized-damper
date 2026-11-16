#!/bin/sh
# Extract deterministic evidence from build logs (no timings). Output is hashed into sealed ledger entries.
export LC_ALL=C; B=build
for f in reentrant.log claims_math.log divequiv.log embedded.log O0.log ma_O0.log w128_O0.log oracle.log bigoracle.log mutate.log fb_O3.log fb_O0.log osc.log; do
  [ -f $B/$f ] || { echo "MISSING $B/$f (run make gates first)" >&2; exit 1; }; done
echo "wad64_trace=$(grep -o 'TRACE_SHA256 [0-9a-f]*' $B/O0.log | head -1 | cut -d' ' -f2)"
echo "wad64_result=$(grep RESULT $B/O0.log)"
echo "cert_pass_count=$(grep -c 'CERT.*PASS' $B/O0.log)"
echo "multiaxis_trace=$(grep -o 'MULTI_TRACE_SHA256 [0-9a-f]*' $B/ma_O0.log | cut -d' ' -f2)"
echo "multiaxis_result=$(grep RESULT $B/ma_O0.log)"
echo "wad128_trace=$(grep -o 'WAD128_TRACE_SHA256 [0-9a-f]*' $B/w128_O0.log | cut -d' ' -f2)"
echo "wad128_result=$(grep RESULT $B/w128_O0.log)"
echo "wad128_t1=$(grep '^T1' $B/w128_O0.log)"
echo "oracle=$(cat $B/oracle.log)"
echo "bigoracle=$(cat $B/bigoracle.log)"
echo "mutants_killed=$(grep -c ': killed' $B/mutate.log)"
echo "mutants_survived=$(grep -c 'SURVIVED' $B/mutate.log)"
echo "mutation_gate=$(grep 'MUTATION GATE' $B/mutate.log)"
echo "divequiv=$(cat $B/divequiv.log)"
echo "embedded=$(grep GATE $B/embedded.log | tr '\n' ' ')"
echo "reentrant=$(tr '\n' ' ' < $B/reentrant.log)"   # sizes are compiler-dependent: reported, not hashed
echo "claims_math=$(grep '^CLAIMS_MATH' $B/claims_math.log)"
echo "float_wad_distinct=$(cat $B/fb_*.log | grep -h WAD_TRACE | sort -u | wc -l | tr -d ' ')"
echo "float_double_distinct=$(cat $B/fb_*.log | grep -h DOUBLE_TRACE | sort -u | wc -l | tr -d ' ')"
echo "float_ref=$(grep REF $B/fb_O3.log)"
echo "float_diff=$(grep max_abs_diff $B/fb_O3.log)"
grep SUMMARY $B/osc.log | sed 's/^/osc: /'
[ -n "$TAMPER_SELFTEST" ] || { [ -f $B/tamper.log ] || { echo "MISSING $B/tamper.log" >&2; exit 1; }; echo "tamper_ok=$(grep -c ': ok' $B/tamper.log) tamper_wrong=$(grep -c WRONG $B/tamper.log) gate=$(grep 'TAMPER GATE' $B/tamper.log)"; }
[ -n "$TAMPER_SELFTEST" ] || { [ -f $B/claims.log ] || { echo "MISSING $B/claims.log" >&2; exit 1; }; grep "^CLAIM" $B/claims.log; [ -f $B/claimstest.log ] || { echo "MISSING $B/claimstest.log" >&2; exit 1; }; echo "claims_selftest=$(grep 'SELFTEST' $B/claimstest.log)"; }
echo "sens_files_sha256=$(cat sweep_data/*.json | sha256sum | cut -d' ' -f1)"
