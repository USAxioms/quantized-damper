#!/bin/sh
# Append a machine-generated ledger entry. Run only after `make gates` passed. No hand-typed values.
set -e; export LC_ALL=C
ev=$(sh tools/collect.sh); evh=$(printf '%s\n' "$ev" | sha256sum | cut -d' ' -f1)
esc=$(printf '%s' "$ev" | sed ':a;N;$!ba;s/\n/\\n/g')
seq=$(( $(wc -l < ledger.jsonl) + 1 )); prev=$(tail -n1 ledger.jsonl | sha256sum | cut -d' ' -f1)
printf '{"seq":%s,"track":"sealed_run","sealed":true,"generator":"tools/seal.sh","evidence":"%s","evidence_sha256":"%s","capsule_src_sha256":"%s","closure_meaning":"all automated gates passed","host_only":true,"prev":"%s"}\n' \
  "$seq" "$esc" "$evh" "$(sh tools/srchash.sh)" "$prev" >> ledger.jsonl
echo "sealed entry seq=$seq evidence_sha256=$evh"
