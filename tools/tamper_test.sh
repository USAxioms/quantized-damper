#!/bin/sh
# Self-test of the ledger verifier. Seals a fresh entry in a temp copy, then every tamper must be rejected.
export LC_ALL=C TAMPER_SELFTEST=1; W=$(mktemp -d); fail=0
cp -r . "$W/base"; (cd "$W/base" && sh tools/seal.sh >/dev/null) || { echo "TAMPER SETUP FAILED"; exit 1; }
chk(){ # name expected(pass|fail) command-to-apply
  rm -rf "$W/c"; cp -r "$W/base" "$W/c"; (cd "$W/c" && eval "$3")
  if [ "$2" = fail ] && diff -rq "$W/base" "$W/c" >/dev/null 2>&1; then echo "TAMPER $1: INEFFECTIVE TEST (edit changed nothing) WRONG"; fail=1; return; fi
  (cd "$W/c" && sh tools/verify_ledger.sh >/dev/null 2>&1); rc=$?
  if [ "$2" = pass ]; then ok=$([ $rc = 0 ] && echo 1 || echo 0); else ok=$([ $rc != 0 ] && echo 1 || echo 0); fi
  if [ "$ok" = 1 ]; then echo "TAMPER $1: ok (expected $2)"; else echo "TAMPER $1: WRONG (rc=$rc, expected $2)"; fail=1; fi; }
chk T0_untouched          pass ":"
chk T1_edit_source        fail "echo '/* x */' >> core/wad18.c"
chk T2_edit_run_log       fail "sed -i 's/mismatches=0/mismatches=1/' build/oracle.log"
chk T3_edit_old_entry     fail "sed -i '5s/\"sha256\":\"[0-9a-f]*\"/\"sha256\":\"deadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeefdeadbeef\"/' ledger.jsonl"
chk T4_hand_append        fail "echo '{\"seq\":99,\"closure\":true,\"prev\":\"x\"}' >> ledger.jsonl"
chk T5_edit_evidence_text fail "sed -i '\$s/mutants_killed=[0-9]*/mutants_killed=99/' ledger.jsonl"
chk T6_edit_sens_result   fail "sed -i 's/\"overflow\":0/\"overflow\":1/' sweep_data/k4_cf1000.json"
chk T7_edit_old_number    fail "sed -i '20s/\"cycles\":1000000/\"cycles\":1000001/' ledger.jsonl"
chk T8_delete_old_entry   fail "sed -i '10d' ledger.jsonl"
rm -rf "$W"; [ $fail = 0 ] && echo "TAMPER GATE PASS" || { echo "TAMPER GATE FAIL"; exit 1; }
