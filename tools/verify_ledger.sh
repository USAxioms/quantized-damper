#!/bin/sh
# Verifies ledger.jsonl: hash chain, per-file hashes, and sequence. Exit 1 on any mismatch.
prev=$(tail -n1 CSL.log | sha256sum | cut -d' ' -f1); n=0; bad=0
while IFS= read -r line; do
  n=$((n+1))
  p=$(printf '%s' "$line" | sed -n 's/.*"prev":"\([0-9a-f]*\)".*/\1/p')
  [ "$p" = "$prev" ] || { echo "CHAIN BREAK at seq $n"; bad=1; }
  f=$(printf '%s' "$line" | sed -n 's/.*"file":"\([^"]*\)".*/\1/p')
  f=$(printf '%s' "$f" | sed 's#^results/sens/#sweep_data/#')   # legacy: entries 1-15 recorded the pre-rename path (CodeOcean reserves /results); ledger is append-only
  if [ -n "$f" ]; then h=$(printf '%s' "$line" | sed -n 's/.*"sha256":"\([0-9a-f]*\)".*/\1/p')
    [ "$(sha256sum "$f" | cut -d' ' -f1)" = "$h" ] || { echo "FILE HASH MISMATCH $f"; bad=1; }; fi
  prev=$(printf '%s\n' "$line" | sha256sum | cut -d' ' -f1)
done < ledger.jsonl
last=$(grep -o '"capsule_src_sha256":"[0-9a-f]*"' ledger.jsonl | tail -1 | cut -d'"' -f4); cur=$(sh tools/srchash.sh)
[ "$last" = "$cur" ] || { echo "SRC HASH MISMATCH: latest ledger entry does not describe the current source (append a new entry)"; bad=1; }
# latest entry must be machine-sealed, and its evidence must equal evidence re-extracted from THIS run's logs
lastline=$(tail -n1 ledger.jsonl)
if printf '%s' "$lastline" | grep -q '"sealed":true'; then
  eh=$(printf '%s' "$lastline" | sed -n 's/.*"evidence_sha256":"\([0-9a-f]*\)".*/\1/p')
  ch=$(sh tools/collect.sh | sha256sum | cut -d' ' -f1)
  et=$(printf '%s\n' "$(printf '%s' "$lastline" | sed -n 's/.*"evidence":"\([^"]*\)".*/\1/p' | sed 's/\\n/\n/g')" | sha256sum | cut -d' ' -f1)
  [ "$et" = "$eh" ] || { echo "EMBEDDED EVIDENCE TEXT DOES NOT MATCH ITS HASH"; bad=1; }
  [ "$eh" = "$ch" ] || { echo "EVIDENCE MISMATCH: sealed evidence differs from this run's logs"; bad=1; }
else echo "LATEST ENTRY NOT MACHINE-SEALED (run make seal)"; bad=1; fi
echo "ledger entries=$n chain_ok=$([ $bad = 0 ] && echo true || echo false) latest_src_hash_matches=$([ "$last" = "$cur" ] && echo true || echo false)"; exit $bad
