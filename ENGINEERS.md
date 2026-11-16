# WAD-18 capsule: engineer's one-pager (measured facts only)

**What it is:** dependency-free C99 fixed-point arithmetic (scale 1e18; int64 core, 128-bit wide type) plus an exact integer
design of discrete critical damping, with a verification harness. `make verify` (~35 s) rebuilds and re-checks everything.

**Measured**
- Freestanding compile (-ffreestanding), no libc/heap symbols. Text (x86-64 -O2): 64-bit core 1397 B, 128-bit 2062 B, damper design 774 B.
- Control step (1 div + 4 mul, driven plant) on a 2.1 GHz Xeon: ~200 ns for WAD-18 vs ~7 ns for double (~30x). Was ~1.75 us before the division rewrite.
- Bit-identical closed-loop traces (1M cycles) across -O0/-O3/-march=native/unsigned-char/ASan+UBSan on this host. The double version of the same loop gave 4 different hashes under FMA / -ffast-math builds.
- Accuracy depends on magnitude: abs. error ~2e-15 flat for WAD-18; double wins at small amplitudes, WAD-18 wins near amplitude 1 (oscillator test, binary128 reference).
- Overflow saturates and sets a flag (loud, not silent).

**Not measured / weak spots**
- No MCU run, no ARM/RISC-V run (cross-ISA identity is argued by construction, unmeasured).
- Default API keeps a global overflow flag (not thread/ISR safe). Use the reentrant `*_f` API (caller-owned flag); built with `-DWAD_NO_GLOBALS` the objects contain no .data/.bss symbols, and an 8-thread test reproduces sequential results exactly. Interrupt-safety and stack depth (design recursion <= ~60 frames) are not measured.
- Division timing is bounded and small but data-dependent (not constant-time).
- 30x slower than hardware double on x86. On an MCU without 64-bit hardware divide the gap is unmeasured.
- Range +-9.22 (int64) / +-1.7e20 (128-bit). Plants beyond that need rescaling.
- One simple plant class (mass-spring-damper, decoupled axes). No coupled dynamics, no hardware-in-the-loop.

**Claim traceability:** `claims.tsv` / `CLAIMS.md` list every claim (23 verified, 2 refuted, 4 unverified, 1 not claimed, 1 not implemented) with the axioms it rests on (C99 semantics, compiler, CPU, SHA-256, POSIX tools, reference oracles, real arithmetic, finite-testing limits). `make verify` re-executes each claim's evidence and rejects cycles, dangling or unverified dependencies, and missing evidence.

**Design-accuracy caveat:** the damping design is exact for the quantized plant. Against the real-valued closed form c = 2*sqrt(km) - k*dt it deviates by up to ~1e-11 (rounded dt^2 amplified by k/m, sqrt sensitivity, scale m/dt); a derived bound holds on 15,864 sampled plants.

**Validation:** exact equivalence tests vs independent references (differential, native int128, 32-bit-limb bignum, retained bitwise reference),
18 injected-fault mutants (+2 argued unreachable), tamper-evident hash-chained evidence ledger with machine-generated entries.
