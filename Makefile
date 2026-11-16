.PHONY: reentrant claims_math all build run cross clean gates verify seal mutate multiaxis wad128 oracle bigoracle floatbase osc divequiv embedded perf
CC ?= cc
SRC = benchmark/bench.c core/wad18.c core/sha256.c breakthrough/r3.c
FLAGS = -std=c99 -Wall -Wextra -pedantic -Werror -fno-fast-math
all: run
build:
	@mkdir -p build
	$(CC) $(FLAGS) -O0 $(SRC) -o build/bench_O0
	$(CC) $(FLAGS) -O3 $(SRC) -o build/bench_O3
run: build
	@./build/bench_O0 | tee build/O0.log
	@./build/bench_O3 > build/O3.log
	@a=$$(grep TRACE_SHA256 build/O0.log); b=$$(grep TRACE_SHA256 build/O3.log); \
	 if [ "$$a" = "$$b" ]; then echo "BIT-EXACT: -O0 == -O3 ($$a)"; else echo "MISMATCH"; exit 1; fi
# Cross-substrate: needs cross compilers + qemu-user installed; otherwise skipped honestly.
cross: build
	@for t in aarch64 riscv64; do \
	 if command -v $$t-linux-gnu-gcc >/dev/null && command -v qemu-$$t >/dev/null; then \
	  $$t-linux-gnu-gcc $(FLAGS) -O2 -static $(SRC) -o build/bench_$$t && \
	  echo "$$t: $$(qemu-$$t build/bench_$$t | grep TRACE_SHA256)"; \
	 else echo "$$t: SKIPPED (toolchain/qemu not installed)"; fi; done
	@echo "host:    $$(./build/bench_O3 | grep TRACE_SHA256)"
clean:
	rm -rf build

build/sens: benchmark/sens.c core/wad18.c core/sha256.c breakthrough/r3.c
	@mkdir -p build
	$(CC) $(FLAGS) -O2 $^ -o $@
# One command: capsule run + sensitivity regeneration (must be byte-identical to stored results) + ledger chain check.
gates: run multiaxis wad128 oracle bigoracle mutate divequiv embedded reentrant claims_math floatbase osc build/sens
	@rm -rf build/s2; sh benchmark/sens.sh build/sens build/s2
	@diff -r sweep_data build/s2 && echo "SENS: regenerated results byte-identical to stored results"
	@sh tools/tamper_test.sh > build/tamper.log; rc=$$?; cat build/tamper.log; exit $$rc
	@sh tools/claims_selftest.sh > build/claimstest.log; rc=$$?; cat build/claimstest.log; [ $$rc = 0 ] || exit $$rc
	@sh tools/check_claims.sh > build/claims.log; rc=$$?; cat build/claims.log; sh tools/check_claims.sh --md > CLAIMS.md; exit $$rc
	@echo "ALL GATES PASS"
verify: gates
	@sh tools/verify_ledger.sh
	@echo "CAPSULE VERIFIED 100%"
# Append a machine-generated evidence entry to ledger.jsonl (only after all gates pass).
seal: gates
	@sh tools/seal.sh

multiaxis: build
	$(CC) $(FLAGS) -O0 benchmark/multiaxis.c core/wad18.c core/sha256.c breakthrough/r3.c -o build/ma_O0
	$(CC) $(FLAGS) -O3 benchmark/multiaxis.c core/wad18.c core/sha256.c breakthrough/r3.c -o build/ma_O3
	@./build/ma_O0 > build/ma_O0.log; ./build/ma_O3 > build/ma_O3.log; cat build/ma_O0.log
	@a=$$(grep MULTI_TRACE build/ma_O0.log); b=$$(grep MULTI_TRACE build/ma_O3.log); \
	 [ "$$a" = "$$b" ] && grep -q "RESULT PASS" build/ma_O0.log && echo "MULTIAXIS BIT-EXACT O0==O3" || { echo "MULTIAXIS MISMATCH/FAIL"; exit 1; }

wad128: build
	$(CC) $(FLAGS) -O0 benchmark/wad128_test.c core/wad18.c core/wad128.c core/sha256.c breakthrough/r3.c breakthrough/r3_128.c -o build/w128_O0
	$(CC) $(FLAGS) -O3 benchmark/wad128_test.c core/wad18.c core/wad128.c core/sha256.c breakthrough/r3.c breakthrough/r3_128.c -o build/w128_O3
	@./build/w128_O0 > build/w128_O0.log; ./build/w128_O3 > build/w128_O3.log; cat build/w128_O0.log
	@a=$$(grep WAD128_TRACE build/w128_O0.log); b=$$(grep WAD128_TRACE build/w128_O3.log); \
	 [ "$$a" = "$$b" ] && grep -q "RESULT PASS" build/w128_O0.log && echo "WAD128 BIT-EXACT O0==O3" || { echo "WAD128 MISMATCH/FAIL"; exit 1; }

oracle: build
	$(CC) $(FLAGS) -O2 benchmark/wad128_oracle.c core/wad18.c core/wad128.c -o build/oracle
	@./build/oracle | tee build/oracle.log; grep -q "PASS\|SKIPPED" build/oracle.log

bigoracle: build
	$(CC) $(FLAGS) -O2 benchmark/wad128_bigoracle.c core/wad18.c core/wad128.c -o build/bigoracle
	@./build/bigoracle | tee build/bigoracle.log; grep -q "PASS" build/bigoracle.log

mutate: build
	@sh tools/mutate.sh > build/mutate.log; rc=$$?; cat build/mutate.log; exit $$rc

# Float baseline: WAD trace must be identical in every build; double trace is REPORTED (it is expected to vary with flags).
floatbase: build
	@for v in "O0:-O0" "O3:-O3" "fma:-O3 -ffp-contract=fast -march=native" "fast:-O3 -ffast-math" "fastfma:-O3 -ffast-math -ffp-contract=fast -march=native -funroll-loops"; do \
	  n=$${v%%:*}; f=$${v#*:}; $(CC) $(FLAGS) $$f benchmark/floatbase.c core/wad18.c core/sha256.c breakthrough/r3.c -o build/fb_$$n -lm || exit 1; \
	  ./build/fb_$$n > build/fb_$$n.log; done
	@w=$$(cat build/fb_*.log | grep -h WAD_TRACE | sort -u | wc -l); d=$$(cat build/fb_*.log | grep -h DOUBLE_TRACE | sort -u | wc -l); \
	 echo "FLOATBASE builds=5 distinct_WAD_hashes=$$w distinct_double_hashes=$$d"; grep -h REF build/fb_O3.log; grep -h max_abs_diff build/fb_O3.log; \
	 [ "$$w" = 1 ] && grep -q efc26cf7f93b1e8820af9ccceade9c4d41447b47e1ca1b0f072c85e7b9f628f8 build/fb_O0.log && echo "FLOATBASE GATE PASS (WAD invariant across builds)" || { echo "FLOATBASE GATE FAIL"; exit 1; }

# Accumulating-error regime (report + overflow gate): undamped oscillator, amplitude sweep vs binary128 reference.
osc: build
	$(CC) $(FLAGS) -O2 benchmark/osc.c core/wad18.c -o build/osc -lm
	@for s in -14 -7 0 2; do ./build/osc $$s x | grep -E "SUMMARY|RATIO"; done > build/osc.log; cat build/osc.log
	@for s in -14 -7 0; do grep "amp=2^$$s " build/osc.log | grep -q "wad_overflow=0" || { echo "OSC GATE FAIL: unexpected overflow at amp 2^$$s"; exit 1; }; done; \
	 grep "amp=2^2 " build/osc.log | grep -q "wad_overflow=1" && echo "OSC GATE PASS (in-range clean; out-of-range loudly flagged)" || { echo "OSC GATE FAIL: out-of-range not flagged"; exit 1; }

# Word-wise division must equal the former bitwise loop on 12M+ cases; negative-subtract counter (proof check) must stay 0.
divequiv: build
	$(CC) $(FLAGS) -O2 -DWAD_TEST_COUNTERS benchmark/div_equiv.c core/wad18.c -o build/div_equiv
	@./build/div_equiv > build/divequiv.log; rc=$$?; cat build/divequiv.log; exit $$rc

# Embedded-readiness: core must compile freestanding (no libc), reference no libc/heap symbols. Sizes recorded.
embedded: build
	@for f in core/wad18.c core/wad128.c breakthrough/r3.c breakthrough/r3_128.c; do \
	  $(CC) $(FLAGS) -O2 -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -c $$f -o build/emb_$$(basename $$f .c).o || exit 1; done
	@bad=$$(nm -u build/emb_*.o | grep -E "malloc|calloc|realloc|free|printf|memcpy|memset|memmove|strlen" || true); \
	 [ -z "$$bad" ] && echo "EMBEDDED GATE PASS: freestanding compile, no libc/heap symbols" > build/embedded.log || { echo "EMBEDDED GATE FAIL: $$bad" > build/embedded.log; cat build/embedded.log; exit 1; }
	@for f in core/wad18.c core/wad128.c breakthrough/r3.c breakthrough/r3_128.c; do \
	  $(CC) $(FLAGS) -O2 -DWAD_NO_GLOBALS -ffreestanding -fno-builtin -fno-stack-protector -fno-pic -c $$f -o build/ng_$$(basename $$f .c).o || exit 1; done
	@bad=$$(nm build/ng_*.o | grep -E " [bBdD] " || true); \
	 [ -z "$$bad" ] && echo "REENTRANT GATE PASS: -DWAD_NO_GLOBALS objects (wad18, wad128, r3, r3_128) have no .data/.bss symbols" >> build/embedded.log || { echo "REENTRANT GATE FAIL: $$bad" >> build/embedded.log; cat build/embedded.log; exit 1; }
	@size build/emb_*.o | awk 'NR>1{print "embedded_text_bytes " $$6 ": " $$1}' >> build/embedded.log; cat build/embedded.log

# Report-only timing (host dependent; not a gate).
perf: build
	$(CC) $(FLAGS) -O2 benchmark/perf.c core/wad18.c -o build/perf -lm
	@./build/perf

# Machine checks behind the mathematical claims (M1-M4): identity spot check, certificates and derived error bound on 15k+ plants.
claims_math: build
	$(CC) $(FLAGS) -O2 benchmark/claims_math.c core/wad18.c breakthrough/r3.c -o build/claims_math -lm
	@./build/claims_math > build/claims_math.log; rc=$$?; cat build/claims_math.log; exit $$rc

# Reentrancy: *_f API vs global wrappers, 8 threads x 3 rounds with per-thread flags (must equal sequential), flag unit cases.
reentrant: build
	$(CC) $(FLAGS) -O2 -pthread benchmark/reentrant_test.c core/wad18.c core/wad128.c breakthrough/r3.c breakthrough/r3_128.c -o build/reentrant
	$(CC) $(FLAGS) -O2 -pthread -DWAD_NO_GLOBALS benchmark/reentrant_test.c core/wad18.c core/wad128.c breakthrough/r3.c breakthrough/r3_128.c -o build/reentrant_ng
	@./build/reentrant > build/reentrant.log; rc=$$?; ./build/reentrant_ng >> build/reentrant.log; rc2=$$?; cat build/reentrant.log; [ $$rc = 0 ] && [ $$rc2 = 0 ]
