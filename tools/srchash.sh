#!/bin/sh
# Hash of all capsule source/build files (excludes ledger, sweep_data, README). LC_ALL=C for stable glob order.
export LC_ALL=C
cat Makefile core/*.c core/*.h breakthrough/* benchmark/*.c benchmark/*.sh tools/*.sh | sha256sum | cut -d' ' -f1
