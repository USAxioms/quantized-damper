### WAD-18 Compute Capsule

### Deterministic Integer Computation, Exact Discrete Critical Damping, and Machine-Verifiable Reproducibility

## 1. System Overview

The WAD-18 Compute Capsule is a zero-dependency C99 implementation built around deterministic integer computation, exact discrete control-system design, independent arithmetic validation, and cryptographically verifiable execution records.

The engineering objective is straightforward: eliminate floating-point variability from the WAD-18 execution path, make numerical behavior deterministic, and provide an executable verification pipeline that checks the implementation against its defined invariants.

The capsule integrates fixed-point arithmetic, extended-precision integer operations, recursive discrete critical-damping design, differential testing, mutation testing, execution-trace comparison, and a hash-chained evidence ledger.

The primary execution target builds the same implementation at two optimization levels, executes one million control cycles, and compares the resulting traces bit for bit.

### Primary execution

```bash
make run
```

This builds at:

- `-O0`
- `-O3`

Both builds execute 1,000,000 control cycles. Their resulting execution traces are compared for bit-identical output.

### Cross-architecture execution

```bash
make cross
```

This target compares AArch64 and RISC-V 64-bit execution using `qemu-user` when available.

If the required emulation environment is not installed, the test reports `SKIPPED`. The pipeline never substitutes an assumed result for an execution that did not occur.

## 2. Core Architecture

### 2.1 WAD-18 Fixed-Point Arithmetic

The `core/` directory contains the WAD-18 arithmetic engine.

WAD-18 uses signed 64-bit integer representation with a fixed scale of \(10^{18}\). Every numerical operation follows explicitly defined integer semantics.

| Property | Implementation |
|---|---|
| Language | C99 |
| Dependencies | Zero external dependencies |
| Numeric representation | Signed int64 |
| Fixed-point scale | \(10^{18}\) |
| Representable real-valued range | Approximately −9.22 to +9.22 |
| Overflow handling | Saturating arithmetic |
| Overflow tracking | Sticky overflow flag |
| Extended arithmetic | Exact 128-bit helpers |
| Cryptographic hashing | SHA-256 |

The arithmetic engine handles overflow explicitly. When an operation exceeds the representable range, the result saturates and the overflow flag remains asserted.

The implementation does not silently wrap arithmetic or introduce floating-point conversions into the WAD-18 execution path.

The core consists of:

- WAD-18 integer arithmetic.
- Exact 128-bit helper operations.
- Saturating arithmetic and overflow management.
- SHA-256 hashing for execution and evidence integrity.

### 2.2 R³ Exact Discrete Critical Damping

The `breakthrough/` directory implements the discrete critical-damping design.

The governing equation is:

\[
(a+w)^2=4w
\]

Solving for the design parameter gives:

\[
a=2\sqrt{w}-w
\]

The implementation uses recursive bisection against an exact 128-bit integer predicate to determine the discrete solution.

This is the central computational breakthrough in this component: the design procedure performs exact integer evaluation without relying on floating-point approximations or intermediate rounding.

Critical damping is established classical control theory. The implemented contribution is the exact, rounding-free discrete design procedure and its associated machine-checkable integer certificates.

The certificate checks are executable and integrated into the benchmark infrastructure.

### 2.3 Benchmark and Certificate Infrastructure

The `benchmark/` directory implements certificate checks C1–C5 and the one-million-cycle hashed execution trace.

The benchmark infrastructure verifies the defined numerical and computational properties of the implementation and produces evidence that can be regenerated and compared against stored results.

## 3. Deterministic Execution

The system is designed around integer-only execution, explicit overflow semantics, and deterministic state transitions.

The implementation uses endian-independent hashing to ensure that byte-order differences do not alter the logical interpretation of the hashed data.

The primary determinism test compares the `-O0` and `-O3` builds after one million control cycles.

The result is a bit-exact comparison of the complete generated traces.

Cross-architecture determinism is also addressed through the optional AArch64/RISC-V execution target.

The distinction between the architecture's deterministic design and the execution environments actually tested is maintained directly in the verification output.

## 4. Verification Pipeline

The complete verification entry point is:

```bash
make verify
```

This is the primary integration gate for the capsule.

It executes the following sequence:

1. Compile the implementation at `-O0` and `-O3`.
2. Execute 1,000,000 control cycles.
3. Generate and compare execution traces.
4. Require bit-identical trace results.
5. Regenerate `sweep_data/`.
6. Compare regenerated JSON outputs against stored reference data.
7. Verify the `ledger.jsonl` cryptographic hash chain.
8. Execute the three-axis multiaxis validation.
9. Execute the 128-bit arithmetic differential tests.
10. Run the native integer arithmetic oracle.
11. Run the independent 32-bit-limb BigOracle.
12. Execute mutation testing.
13. Run the applicable integrity and sealing checks.
14. Verify that the latest ledger entry references the current source.

Any required verification failure or mismatch produces a nonzero exit status.

### 4.1 Evidence Architecture

The capsule maintains three principal evidence artifacts.

| Artifact | Engineering function |
|---|---|
| `CSL.log` | Genesis Cognitive State Ledger |
| `ledger.jsonl` | Cryptographically chained execution and verification history |
| `sweep_data/*.json` | Generated numerical sweep and validation results |

The ledger records execution history, verification outcomes, and iteration histories, including failures.

This makes the development and verification history inspectable rather than reducing the project to a collection of final pass/fail statements.

## 5. Extended Arithmetic and Independent Validation

### 5.1 Multi-Axis R³ Validation

The `multiaxis` implementation evaluates three axes using different \(m\) and \(k\) parameters through R³.

This extends the verification beyond a single numerical configuration and is included directly in `make verify`.

### 5.2 WAD-18 128-Bit Arithmetic

The `wad128` implementation extends the arithmetic architecture through 128-bit limbs.

Implementation files:

```text
core/wad128.*
breakthrough/r3_128.*
```

The extended representation supports an approximate real-valued range of:

\[
\pm 1.7\times10^{20}
\]

The 128-bit implementation is differential-tested against the 64-bit WAD-18 core over their overlapping representable domains.

Compilation uses:

```bash
-Werror
```

Warnings are treated as build failures.

This makes the extended arithmetic implementation part of the same executable verification architecture rather than an isolated numerical experiment.

### 5.3 Independent Arithmetic Oracles

Three validation mechanisms provide complementary coverage.

**Oracle — Native 128-Bit Reference**

Uses native integer 128-bit arithmetic as the reference implementation.

**BigOracle — Independent Arbitrary-Precision Reference**

Implements arithmetic independently through 32-bit limbs and includes explicit tie-handling tests.

**Mutation — Fault-Injection Validation**

Introduces deliberate implementation defects to test whether the verification infrastructure detects them.

These three mechanisms serve different purposes: reference comparison, independent arithmetic validation, and verification-suite sensitivity.

### 5.4 Division Optimization

The unsigned division implementation skips leading zero bits while preserving exact arithmetic equivalence.

This reduces unnecessary division iterations without changing the numerical result.

Measured performance improvement:

**Approximately 1.7× on the test host.**

The optimized implementation is included in the verification pipeline.

## 6. Comparative Numerical Experiments

### 6.1 Floating-Point Baseline

The `floatbase` experiment compares double-precision arithmetic against WAD-18 across five build configurations.

The experiment produces an important engineering result:

- WAD-18 maintains its defined arithmetic invariants across the tested configurations.
- Double-precision execution produces four different hashes.
- Double precision demonstrates greater numerical accuracy against the long-double reference used in the experiment.

This establishes a distinction between two separate numerical properties:

**Deterministic execution and numerical accuracy are not interchangeable.**

WAD-18 is designed to provide deterministic arithmetic behavior. Double precision can provide greater approximation accuracy for particular numerical workloads.

The experiment makes that distinction measurable rather than theoretical.

### 6.2 Accumulating-Error Oscillator

The `osc` experiment compares an accumulating-error oscillator against a binary128 reference.

The results establish the following behavior:

- WAD-18 performs better under the tested conditions at magnitudes above approximately 0.1–1.
- Double precision performs better at smaller magnitudes.
- WAD-18 overflow is explicitly detected and flagged.

The experiment evaluates accumulated numerical error across the tested operating ranges and provides a direct comparison between fixed-point and floating-point execution.

Both `floatbase` and `osc` are included in `make verify`.

## 7. Mutation Testing

Mutation testing is integrated into the verification architecture to establish whether the test suite detects deliberately introduced defects.

The mutation gate has undergone successive expansions.

The recorded findings include:

- Seven killed mutants, including a carry-handling defect reachable only through specially constructed operands.
- The carry defect is tested in `benchmark/wad128_bigoracle.c`.
- Two additional mutants were classified as unreachable under the evaluated conditions.
- The latest expanded mutation gate contains 18 mutants, including overflow-flag handling.

The gate uses fail-fast behavior. Once a mutant is detected, execution stops; surviving mutants proceed through the full applicable test sequence.

The purpose is to test the verification infrastructure itself, not merely the arithmetic implementation.

## 8. Source Integrity and Evidence Sealing

The F7 follow-up introduced machine-generated evidence collection and ledger sealing.

This adds a source-integrity requirement to the verification pipeline.

### 8.1 Source Hash Verification

The latest ledger entry must contain a `capsule_src_sha256` value matching the current capsule source.

The check is implemented through:

```text
tools/srchash.sh
```

This connects the verification record to the source version being executed.

A source mismatch prevents successful verification.

### 8.2 Machine-Generated Sealing

Two primary commands control the sealing workflow.

**Full verification**

```bash
make verify
```

The complete workflow includes:

- All designated automated gates.
- Tamper-detection self-tests.
- Ledger hash-chain verification.
- Latest-entry machine-sealing validation.
- Source-to-ledger hash verification.
- Evidence re-extraction from the current execution logs.

Reported execution time:

Approximately 51 seconds on the test host.

**Generate and append a new sealed entry**

```bash
make seal
```

This command:

1. Runs the designated verification gates.
2. Collects evidence from the resulting execution logs.
3. Generates a machine-produced ledger entry.
4. Appends the entry to `ledger.jsonl`.

Implementation:

```text
tools/collect.sh
tools/seal.sh
```

Run `make seal` following every source change.

### 8.3 Ledger Provenance

Entries 1–26 predate the machine-generated sealing workflow.

Values beginning with entry 16 were manually transcribed from execution logs.

The ledger preserves the recorded sequence and cryptographic integrity of its entries. The historical manually transcribed values remain identifiable as pre-sealing records.

The sealing architecture protects against accidental modification and unauthorized alteration by parties without unrestricted access to the capsule.

## 9. Claim Register

The authoritative claim definitions are maintained in:

```text
claims.tsv
```

The human-readable claim documentation is generated through:

```bash
make gates
```

Generated output:

```text
CLAIMS.md
```

The claim register connects each documented system property to its corresponding verification requirements.

This provides a centralized engineering reference for the capsule's capabilities, invariants, and verification evidence.

The reported cold-start execution time for `make verify` is approximately 35 seconds on a single-core host, within the 60-second execution budget.

The sealing workflow has also been measured at approximately 51 seconds on the test host.

## 10. Reentrancy and Explicit State Ownership

The implementation supports reentrant operation through caller-owned overflow state.

Supported interfaces:

```c
wad_*_f
w128_*_f
r3_design_damper_f
```

These interfaces accept caller-owned overflow-flag pointers.

Each execution context can therefore maintain its own overflow state without depending on a shared global flag.

To eliminate the global-flag compatibility wrappers, define:

```c
WAD_NO_GLOBALS
```

This configuration provides explicit state ownership and supports integration into systems requiring reentrant arithmetic operations.

The mutation-testing gate includes 18 mutants covering arithmetic behavior, overflow handling, and associated state-management conditions.

## 11. Engineering Characteristics

The completed capsule integrates the following capabilities:

| Engineering capability | Implementation |
|---|---|
| Deterministic fixed-point arithmetic | WAD-18 |
| Explicit overflow management | Saturating arithmetic and sticky flags |
| Extended integer precision | WAD-18 128-bit limbs |
| Exact discrete critical damping | Recursive integer bisection |
| Machine-checkable certificates | C1–C5 |
| Optimization-level determinism | `-O0` / `-O3` trace comparison |
| Long-duration execution validation | 1,000,000 control cycles |
| Multi-axis validation | Three-axis R³ implementation |
| Independent arithmetic validation | Oracle and BigOracle |
| Fault-detection validation | 18-mutant gate |
| Numerical comparison | Floatbase and oscillator experiments |
| Execution evidence | SHA-256 hashed traces |
| Historical verification records | CSL and `ledger.jsonl` |
| Source integrity | `capsule_src_sha256` |
| Machine-generated sealing | `make seal` |
| Reentrant execution | Caller-owned overflow interfaces |
| Cross-architecture testing | AArch64/RISC-V through QEMU |

## 12. Build and Execution Reference

| Command | Function |
|---|---|
| `make run` | Builds and runs the optimization-level determinism test |
| `make cross` | Executes the optional cross-architecture comparison |
| `make verify` | Runs the complete verification pipeline |
| `make seal` | Runs verification and appends a machine-generated ledger entry |
| `make gates` | Generates the claim documentation |

### Principal directories

```text
core/
    WAD-18 arithmetic
    128-bit helpers
    SHA-256
    Extended arithmetic

breakthrough/
    Exact discrete critical-damping design
    Recursive integer bisection
    128-bit R³ implementation

benchmark/
    C1-C5 certificates
    Execution traces
    Arithmetic validation
    BigOracle tests

sweep_data/
    Generated numerical sweep results

tools/
    Source hashing
    Evidence collection
    Ledger sealing
```

### Principal evidence files

```text
CSL.log
ledger.jsonl
claims.tsv
CLAIMS.md
```

## 13. Conclusion

The WAD-18 Compute Capsule brings deterministic numerical computation, exact discrete control-system design, independent arithmetic validation, mutation testing, and machine-generated evidence into one integrated C99 implementation.

The architecture is built around a clear engineering principle: numerical behavior must be defined, execution must be reproducible, defects must be detectable, and verification evidence must be tied to the implementation that produced it.

WAD-18 supplies the deterministic arithmetic foundation. The extended 128-bit implementation expands its computational range. The discrete critical-damping implementation demonstrates exact integer-based design. Independent oracles and mutation testing exercise the arithmetic and the verification infrastructure. The execution ledger and sealing workflow connect the resulting evidence to the source.

This is not simply a fixed-point arithmetic library or a collection of numerical benchmarks.

It is an executable computational verification system, with defined invariants, reproducible execution, independent validation paths, and an integrated evidence architecture.

**The implementation makes its numerical behavior executable, its verification reproducible, and its engineering claims traceable to machine-generated evidence.**
