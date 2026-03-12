# UB TrafficGen MPI/MTP Stabilization Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Stabilize `UbTrafficGen` under MTP multithreading, define the correct multi-process DAG boundary, and add regression coverage that distinguishes local-threading bugs from cross-rank dependency gaps.

**Architecture:** Treat link and remote-link as pure serialized-`Packet` carriers and keep all task/DAG semantics inside unified-bus runtime code. First lock down reproducible oracles for the two verified failures: local MTP dependent-task crash and MPI cross-rank phase dependency mismatch. Then fix process-local shared-state races in `UbTrafficGen`, and finally make cross-rank DAG behavior explicit: either reject unsupported cross-rank phase dependencies early or add a dedicated completion propagation mechanism.

**Tech Stack:** ns-3 core/test framework, unified-bus module, MPI (`mpirun`), MTP (`HybridSimulatorImpl` / `MtpInterface`), C++17

---

## Verified Problem Boundary

- Verified on 2026-03-12 in current `feat-mpi` branch:
  - Local dependent DAG case on one process:
    - `build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=/tmp/ub-local-phase-7VUBi0 --mtp-threads=2 --test`
    - repeated run reproduced intermittent `RC=139`
  - Same case is stable without multithreading:
    - `--mtp-threads=1` passes 10/10
    - no `--mtp-threads` passes
  - MPI hybrid case with cross-rank phase dependency shows rank-local completion mismatch:
    - `mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=/tmp/ub-mpi-cross-phase-qT1Q2q --mtp-threads=2 --stop-ms=1 --test`
    - observed mixed `PASSED` / `FAILED`
- Current evidence points to two separate issues:
  - process-local thread safety bug in `UbTrafficGen`
  - missing or undefined cross-rank DAG completion semantics
- Input note verified from `UbUtils::SetRecord(...)`:
  - `dependOnPhases` is currently parsed by whitespace tokenization, not `;`
  - regression CSV examples in this plan therefore use `10 20`, not `10;20`

## File Map

- Modify: `src/unified-bus/model/ub-traffic-gen.h`
- Modify: `src/unified-bus/model/ub-traffic-gen.cc`
- Modify: `src/unified-bus/examples/ub-quick-example.cc`
- Modify: `src/unified-bus/test/ub-test.cc`
- Modify: `src/unified-bus/model/ub-utils.h`
- Modify: `src/unified-bus/model/ub-utils.cc`
- Optional modify after decision: `src/unified-bus/model/ub-app.cc`
- Optional create only if test size becomes unreasonable: `src/unified-bus/test/ub-trafficgen-test-helper.h`

## Chunk 1: Lock down failing oracles

### Task 1: Add reusable dependent-task case builders

**Files:**
- Modify: `src/unified-bus/test/ub-test.cc`

- [ ] **Step 1: Add helper that copies a base quick-example case and rewrites `traffic.csv`**

Implement a small helper in `src/unified-bus/test/ub-test.cc` next to `CopyCaseDirWithoutFile(...)` that:
- copies a source case directory into a temp directory
- rewrites `traffic.csv` with caller-provided content
- returns the temp directory path

- [ ] **Step 2: Add helper to run quick-example and capture exit code + output**

Reuse `RunQuickExampleCommand(...)` pattern. If needed, add a variant that accepts:
- full absolute case path
- `commandPrefix`
- `extraArgs`

- [ ] **Step 3: Add a local dependent-DAG system test that is expected to pass in single-thread mode**

Use a temp case derived from `scratch/ub-local-hybrid-minimal` with:

```csv
taskId,sourceNode,destNode,dataSize(Byte),opType,priority,delay,phaseId,dependOnPhases
0,0,3,4096,URMA_WRITE,7,10ns,10,
1,3,0,4096,URMA_WRITE,7,10ns,20,
2,0,3,4096,URMA_WRITE,7,10ns,30,10 20
```

Run:

```bash
build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=<temp> --mtp-threads=1 --test
```

Expected:
- exit code `0`
- output contains `TEST : 00000 : PASSED`

- [ ] **Step 4: Add a deterministic local dependent-DAG MTP RED oracle**

Use a temp case derived from `scratch/ub-local-hybrid-minimal` with:
- task `0` producing phase `10`
- task `1` producing phase `20`
- a fanout of follow-up tasks (`2..40`) all depending on `10 20`

Run:

```bash
build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=<temp> --mtp-threads=2 --test
```

Test assertion target:
- exit code `0`
- output contains `TEST : 00000 : PASSED`

Expected before fix:
- this test fails deterministically in current tree
- currently observed failure shape is `RC=139` / segmentation fault

Expected after fix:
- test passes without changing its assertions

- [ ] **Step 5: Add a cross-rank phase-dependency system test**

Use a temp case derived from `scratch/ub-mpi-hybrid-minimal` with:

```csv
taskId,sourceNode,destNode,dataSize(Byte),opType,priority,delay,phaseId,dependOnPhases
0,0,3,4096,URMA_WRITE,7,10ns,10,
1,3,0,4096,URMA_WRITE,7,10ns,20,10
```

Run:

```bash
mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=<temp> --mtp-threads=2 --stop-ms=1 --test
```

Test assertion target:
- no rank prints `TEST : 00000 : FAILED`
- merged output contains `TEST : 00000 : PASSED`

Expected before decision:
- this test fails in current tree because local completion and global DAG expectation are mismatched

- [ ] **Step 6: Commit oracle-only checkpoint**

```bash
git add src/unified-bus/test/ub-test.cc
git commit -m "test: add ub trafficgen mpi/mtp regression oracles"
```

## Chunk 2: Fix process-local thread safety in `UbTrafficGen`

### Task 2: Protect shared DAG state

**Files:**
- Modify: `src/unified-bus/model/ub-traffic-gen.h`
- Modify: `src/unified-bus/model/ub-traffic-gen.cc`
- Modify: `src/unified-bus/examples/ub-quick-example.cc`

- [ ] **Step 1: Add a mutex for all mutable DAG/runtime state**

Protect:
- `m_tasks`
- `m_dependencies`
- `m_dependents`
- `m_taskStates`
- `m_readyTasks`
- `m_dependOnPhasesToTaskId`

with one coarse lock first. Do not start with fine-grained locks.

- [ ] **Step 2: Add read-only snapshot/query helpers**

Add explicit APIs instead of direct field access from `ub-quick-example`:
- `GetCompletedTaskCount()`
- `IsCompleted()`
- `GetTaskById(...)`

If returning containers is necessary, return copies/snapshots, not references.

- [ ] **Step 3: Refactor `MarkTaskCompleted()` into lock + unlock phases**

Inside lock:
- validate state transition `RUNNING -> COMPLETED`
- compute newly ready tasks
- move ready task ids into a local vector

Outside lock:
- schedule those tasks

Do not call `Simulator::ScheduleWithContext(...)` while holding the mutex.

- [ ] **Step 4: Refactor `ScheduleNextTasks()` so it drains ready tasks safely**

Pattern:
- lock
- collect all currently ready task ids and set them to `RUNNING`
- copy needed `TrafficRecord`
- unlock
- schedule `UbApp::SendTraffic(...)`

- [ ] **Step 5: Stop direct unsynchronized reads in progress monitor**

Change `CheckNoProgress(...)` in `src/unified-bus/examples/ub-quick-example.cc` to use new `UbTrafficGen` query APIs instead of iterating `m_taskStates` directly.

- [ ] **Step 6: Run focused tests**

Run:

```bash
cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target unified-bus-test ub-quick-example
build/utils/ns3.44-test-runner-default --suite=unified-bus-examples --verbose
```

And rerun the local dependent-DAG MTP stress command from Chunk 1.

- [ ] **Step 7: Commit thread-safety checkpoint**

```bash
git add src/unified-bus/model/ub-traffic-gen.h src/unified-bus/model/ub-traffic-gen.cc src/unified-bus/examples/ub-quick-example.cc src/unified-bus/test/ub-test.cc
git commit -m "fix: serialize ub trafficgen state transitions"
```

## Chunk 3: Decide and enforce cross-rank DAG semantics

### Task 3: Choose one supported contract

**Files:**
- Modify: `src/unified-bus/examples/ub-quick-example.cc`
- Modify: `src/unified-bus/model/ub-utils.cc`
- Modify: `src/unified-bus/test/ub-test.cc`
- Optional modify: `src/unified-bus/model/ub-traffic-gen.h`
- Optional modify: `src/unified-bus/model/ub-traffic-gen.cc`
- Optional modify: `src/unified-bus/model/ub-app.cc`

- [ ] **Step 1: Make an explicit product decision**

Pick exactly one:

- **Option A:** quick-example currently supports only rank-local phase dependencies
- **Option B:** quick-example must support cross-rank phase dependencies via explicit completion propagation

Do not implement both in the same patch series.

- [ ] **Step 2A: If choosing Option A, fail fast on unsupported cross-rank dependencies**

Implementation direction:
- detect when a task owned by rank X depends on a phase produced only by tasks owned by rank Y
- abort early with a clear error message before simulation starts

Expected test:
- cross-rank dependency system test fails deterministically with explicit explanation

- [ ] **Step 2B: If choosing Option B, add a minimal cross-rank completion protocol**

Implementation direction:
- when a task completes on one rank, propagate completion information to dependent-task owners
- keep message payload at task/phase completion level, not packet semantic level
- reuse ns-3/MPI scheduling path; do not overload link-level packet payload semantics

Expected test:
- cross-rank dependency system test passes deterministically without `--stop-ms` escape hatch

- [ ] **Step 3: Commit semantic-boundary checkpoint**

```bash
git add src/unified-bus/examples/ub-quick-example.cc src/unified-bus/model/ub-utils.cc src/unified-bus/test/ub-test.cc src/unified-bus/model/ub-traffic-gen.h src/unified-bus/model/ub-traffic-gen.cc src/unified-bus/model/ub-app.cc
git commit -m "feat: define quick example cross-rank task dependency contract"
```

## Chunk 4: Clean up secondary shared-state risks

### Task 4: Stabilize process-local trace helpers

**Files:**
- Modify: `src/unified-bus/model/ub-utils.h`
- Modify: `src/unified-bus/model/ub-utils.cc`
- Modify: `src/unified-bus/test/ub-test.cc`

- [ ] **Step 1: Add synchronization for trace file map and stream writes**

Protect:
- `trace_path`
- `files`
- lazy file-open path in `PrintTraceInfo(...)`
- lazy file-open path in `PrintTraceInfoNoTs(...)`

- [ ] **Step 2: Keep the change minimal**

Do not redesign the trace subsystem. Only make existing per-process shared state safe enough for MTP.

- [ ] **Step 3: Add a regression that exercises quick-example with traces enabled in MTP**

Run one small local case and one small MPI+MTP case with trace-related globals enabled, then assert:
- no crash
- output files exist

- [ ] **Step 4: Commit trace-safety checkpoint**

```bash
git add src/unified-bus/model/ub-utils.h src/unified-bus/model/ub-utils.cc src/unified-bus/test/ub-test.cc
git commit -m "fix: protect ub trace helpers in mtp mode"
```

## Verification Matrix

- [ ] **Build targeted binaries**

```bash
cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target unified-bus-test ub-quick-example
```

- [ ] **Run existing example suite**

```bash
build/utils/ns3.44-test-runner-default --suite=unified-bus-examples --verbose
```

- [ ] **Run deterministic local dependent-DAG MTP verification**

```bash
build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=<temp-local-fanout-case> --mtp-threads=2 --test
```

Expected:
- after fix, exit code `0`
- output contains `TEST : 00000 : PASSED`

- [ ] **Run MPI baseline**

```bash
mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-mpi-hybrid-minimal --mtp-threads=2 --test
```

- [ ] **Run cross-rank dependency verification**

```bash
mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=<temp-mpi-case> --mtp-threads=2 --test
```

Expected:
- Option A: explicit, deterministic unsupported-case failure
- Option B: deterministic pass

## Exit Criteria

- Local dependent-DAG fanout case passes under `--mtp-threads=2`
- Existing `unified-bus-examples` suite still passes
- Cross-rank phase dependency behavior is explicit and regression-covered
- No test relies on link-layer parsing of high-level protocol semantics
- Checkpoint commits exist for: oracle, thread-safety, cross-rank semantics, trace-safety
