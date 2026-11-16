#!/bin/sh
# usage: sens.sh <sens-binary> <outdir>   (avoid shell overflow: factor = f * 1e15)
E=1000000000000000000; U=1000000000000000; mkdir -p "$2"
for k in 1 4 9; do for f in 500 900 1000 1100 2000; do
  "$1" k${k}_cf${f} $((k*E)) $((f*U)) > "$2/k${k}_cf${f}.json" || exit 1; done; done
