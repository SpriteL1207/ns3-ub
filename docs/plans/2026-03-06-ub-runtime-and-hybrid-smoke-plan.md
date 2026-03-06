# Unified-Bus Runtime Fixes and Hybrid Smoke Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Stabilize `ub-quick-example` runtime behavior and add a minimal `ub-hybrid-smoke` example that proves `ns-3-ub` can enter the `MPI + MTP` hybrid framework with 2 ranks.

**Architecture:** Keep `ub-quick-example` focused on single-process and `MTP` use. Fix its trace-directory and command-line handling without expanding it into a distributed example. Add a separate `ub-hybrid-smoke` path built only when `ENABLE_MPI` and `ENABLE_MTP` are enabled; use a thin MPI bridge and a minimal smoke app/example to validate cross-rank packet delivery and completion, without pulling the existing `UbTrafficGen` CSV/DAG loader into phase 1.

**Tech Stack:** ns-3 C++, `mtp`, `mpi`, `HybridSimulatorImpl`, `MpiInterface`, `GrantedTimeWindowMpiInterface`, `CommandLine`, `TestSuite`, `ExampleAsTestCase`.

---

## Current Baseline

- `MTP` example link failure is already fixed by conditionally linking `libmtp` in `src/unified-bus/examples/CMakeLists.txt`.
- Remaining runtime issue 1: `CreateTraceDir()` aborts when `runlog` does not exist.
- Remaining runtime issue 2: `ub-quick-example` treats `argv[1]` as the case path, so `--mtp-threads=2` is misread if passed before the positional case path.
- Current distributed limitation 1: local `ub` example path has no `MpiInterface::Enable(...)` / hybrid entry.
- Current distributed limitation 2: `unified-bus` has no MPI-aware remote link or adapter for cross-rank delivery.
- Current distributed limitation 3: current `UbUtils`/`UbTrafficGen` configuration pipeline assumes single-process global state and is out of scope for phase 1 hybrid smoke.

## Non-Goals for This Plan

- Do not make `UbTrafficGen` fully distributed in this phase.
- Do not make `ub-quick-example` itself the hybrid example in this phase.
- Do not generalize the first remote-link implementation beyond what is needed for a 2-rank smoke test.
- Do not optimize performance yet; correctness and clean bring-up come first.

### Task 1: Fix `ub-quick-example` runtime bring-up

**Files:**
- Modify: `src/unified-bus/model/ub-utils.h`
- Modify: `src/unified-bus/model/ub-utils.cc`
- Modify: `src/unified-bus/examples/ub-quick-example.cc`
- Test: `src/unified-bus/test/ub-test.cc`

**Step 1: Write the failing test for missing `runlog` handling**

Add a focused unit test in `src/unified-bus/test/ub-test.cc` that prepares a temporary case directory without a `runlog` subdirectory, invokes the extracted trace-dir helper or `UbUtils` setup path, and asserts that the directory is created instead of aborting.

**Step 2: Run the focused test to verify it fails**

Run: `./ns3 build unified-bus-test && ./test.py -s unified-bus`
Expected: the new test fails before the fix because missing `runlog` is treated as a fatal error.

**Step 3: Implement the minimal trace-dir fix**

- In `src/unified-bus/model/ub-utils.cc`, change `CreateTraceDir()` so missing `runlog` is not treated as an error.
- Preferred behavior:
  - derive the case directory from the configured case path
  - if `runlog` exists, remove it
  - if `runlog` does not exist, continue normally
  - recreate `runlog`
- If needed for testability, extract a small helper in `src/unified-bus/model/ub-utils.h` / `src/unified-bus/model/ub-utils.cc` that performs directory preparation without requiring full simulation state.

**Step 4: Make command-line parsing unambiguous**

In `src/unified-bus/examples/ub-quick-example.cc`:
- add an explicit option such as `--case-path=<path>`
- keep the default path as `scratch/2nodes_single-tp`
- preserve positional fallback only for backward compatibility if it is simple to keep
- after `CommandLine::Parse`, resolve the case path from the explicit option first, then fall back to a positional path only if no option was provided

**Step 5: Run focused validation for the runtime fix**

Run:
- `./ns3 build src/unified-bus/examples/ub-quick-example unified-bus-test`
- `build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/2nodes_single-tp`
- `build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/2nodes_single-tp --mtp-threads=2`
Expected:
- no abort when `runlog` is absent
- both commands complete successfully
- output files appear under `scratch/2nodes_single-tp/output`

**Step 6: Commit**

```bash
git add src/unified-bus/model/ub-utils.h src/unified-bus/model/ub-utils.cc src/unified-bus/examples/ub-quick-example.cc src/unified-bus/test/ub-test.cc
git commit -m "fix: stabilize unified-bus runtime startup"
```

### Task 2: Add a minimal MPI-aware `ub` bridge layer

**Files:**
- Modify: `src/unified-bus/CMakeLists.txt`
- Create: `src/unified-bus/model/ub-remote-link.h`
- Create: `src/unified-bus/model/ub-remote-link.cc`
- Modify: `src/unified-bus/model/ub-port.h`
- Modify: `src/unified-bus/model/ub-port.cc`
- Reference: `src/mpi/model/mpi-interface.h`
- Reference: `src/mpi/examples/simple-hybrid.cc`

**Step 1: Write the failing design-level smoke target**

Create the smallest possible compile target wiring for a future `ub-hybrid-smoke` example in `src/unified-bus/CMakeLists.txt` and `src/unified-bus/examples/CMakeLists.txt`, guarded by `ENABLE_MPI` and `ENABLE_MTP`, but do not implement the bridge yet.

**Step 2: Build to verify the bridge is still missing**

Run: `./ns3 configure --enable-mpi --enable-mtp --enable-examples --enable-tests && ./ns3 build src/unified-bus/examples/ub-hybrid-smoke`
Expected: build fails because the remote-link implementation does not exist yet.

**Step 3: Implement the minimal remote-link abstraction**

Design `ub-remote-link` to do exactly this:
- hold the remote rank id
- hold the remote node id and remote device id needed for delivery
- expose a send method that serializes a `Packet` handoff through `MpiInterface::SendPacket(...)`
- expose a receive-side delivery hook that forwards the packet to `UbPort::Receive(...)`

Keep scope tight:
- only support the packet shape and timing semantics needed by the smoke example
- do not fold this into `UbLink` yet
- do not attempt generalized distributed topology parsing yet

**Step 4: Integrate `UbPort` with the new bridge**

In `src/unified-bus/model/ub-port.h` and `src/unified-bus/model/ub-port.cc`:
- add the smallest extension required to distinguish local-link delivery from remote-link delivery
- preserve current local behavior for existing examples
- make the new path opt-in and explicit

**Step 5: Build the module and examples**

Run: `./ns3 build src/unified-bus/examples/ub-quick-example src/unified-bus/examples/ub-hybrid-smoke`
Expected: both examples compile on an MPI-enabled machine; existing `ub-quick-example` behavior remains unchanged.

**Step 6: Commit**

```bash
git add src/unified-bus/CMakeLists.txt src/unified-bus/model/ub-remote-link.h src/unified-bus/model/ub-remote-link.cc src/unified-bus/model/ub-port.h src/unified-bus/model/ub-port.cc
git commit -m "feat: add minimal unified-bus mpi bridge"
```

### Task 3: Create `ub-hybrid-smoke` example

**Files:**
- Create: `src/unified-bus/examples/ub-hybrid-smoke.cc`
- Modify: `src/unified-bus/examples/CMakeLists.txt`
- Reference: `src/mpi/examples/simple-hybrid.cc`
- Reference: `src/unified-bus/model/ub-app.h`
- Reference: `src/unified-bus/model/ub-controller.h`
- Reference: `src/unified-bus/model/ub-port.h`
- Optional Create: `src/unified-bus/model/ub-hybrid-smoke-app.h`
- Optional Create: `src/unified-bus/model/ub-hybrid-smoke-app.cc`

**Step 1: Write the failing smoke example skeleton**

Create `src/unified-bus/examples/ub-hybrid-smoke.cc` with:
- `MtpInterface::Enable(...)`
- `MpiInterface::Enable(&argc, &argv)`
- rank detection via `MpiInterface::GetSystemId()`
- explicit creation of one local `DEVICE` node per rank
- rank 0 sender setup and rank 1 receiver setup
- placeholder completion logging in `--test` mode

**Step 2: Build and run to verify it fails before end-to-end delivery exists**

Run on an MPI-enabled machine:
- `./ns3 build src/unified-bus/examples/ub-hybrid-smoke`
- `./ns3 run ub-hybrid-smoke --command-template "mpiexec -n 2 %s --test"`
Expected: starts but does not yet complete a full smoke pass until the end-to-end hook is implemented.

**Step 3: Implement the smallest end-to-end smoke flow**

Choose one thin driving mechanism and keep it simple:
- either reuse `UbApp` only if it can be driven with one minimal request cleanly
- or add `ub-hybrid-smoke-app` to send one packet/request on rank 0 and emit one completion signal after rank 1 receives and acknowledges it

Success path must prove:
- rank 0 initializes
- rank 1 initializes
- one cross-rank `ub` packet/request is sent
- rank 1 receives it
- rank 0 observes completion
- simulation exits cleanly

**Step 4: Add deterministic `--test` output**

In `src/unified-bus/examples/ub-hybrid-smoke.cc`, add stable output lines prefixed with `TEST` so they can be compared or sorted by the existing MPI test harness.

Example target output shape:
- `TEST Rank 0 initialized`
- `TEST Rank 1 initialized`
- `TEST Cross-rank send complete`
- `TEST PASS`

**Step 5: Validate manually on an MPI-enabled host**

Run:
- `./ns3 configure --enable-mpi --enable-mtp --enable-examples --enable-tests`
- `./ns3 build src/unified-bus/examples/ub-hybrid-smoke`
- `./ns3 run ub-hybrid-smoke --command-template "mpiexec -n 2 %s --test"`
Expected: both ranks run, sorted `TEST` lines appear, and the process exits with code 0.

**Step 6: Commit**

```bash
git add src/unified-bus/examples/ub-hybrid-smoke.cc src/unified-bus/examples/CMakeLists.txt src/unified-bus/model/ub-hybrid-smoke-app.h src/unified-bus/model/ub-hybrid-smoke-app.cc
git commit -m "feat: add unified-bus hybrid smoke example"
```

### Task 4: Add MPI regression coverage for the new smoke example

**Files:**
- Modify: `src/mpi/test/mpi-test-suite.cc`
- Create: `src/mpi/test/mpi-example-ub-hybrid-smoke-2.reflog`

**Step 1: Add the failing regression registration**

Register a new `MpiTestSuite` entry in `src/mpi/test/mpi-test-suite.cc` for `ub-hybrid-smoke` with `2` ranks.

**Step 2: Run the MPI smoke test to verify the reflog is missing or output mismatches**

Run on an MPI-enabled machine:
- `./ns3 build test-runner`
- `./test.py -s mpi-example-ub-hybrid-smoke-2`
Expected: failure until the reference output is created and the example output is stable.

**Step 3: Create the minimal reference log**

Add `src/mpi/test/mpi-example-ub-hybrid-smoke-2.reflog` with the sorted `TEST` lines emitted by the smoke example.

**Step 4: Re-run the MPI regression**

Run: `./test.py -s mpi-example-ub-hybrid-smoke-2`
Expected: PASS on an MPI-enabled machine.

**Step 5: Commit**

```bash
git add src/mpi/test/mpi-test-suite.cc src/mpi/test/mpi-example-ub-hybrid-smoke-2.reflog
git commit -m "test: add unified-bus hybrid mpi smoke regression"
```

### Task 5: Document usage and known phase-1 limits

**Files:**
- Modify: `UNISON_README.md`
- Optional Modify: `README.md`
- Optional Modify: `README_en.md`

**Step 1: Add the new smoke example command path**

Document:
- how to configure with `--enable-mpi --enable-mtp --enable-examples`
- how to build `ub-hybrid-smoke`
- how to run with `mpiexec -n 2`
- that phase 1 validates the hybrid framework entry and one minimal cross-rank `ub` flow only

**Step 2: Document current non-goals explicitly**

State clearly that phase 1 does not yet make the CSV/DAG loader or `UbTrafficGen` fully distributed.

**Step 3: Verify docs against the implementation commands**

Run the exact documented commands on an MPI-enabled machine and confirm they match the real target names.

**Step 4: Commit**

```bash
git add UNISON_README.md README.md README_en.md
git commit -m "docs: describe unified-bus hybrid smoke flow"
```

## Validation Matrix

### Local validation in the current environment

Run:
- `./ns3 configure --enable-mtp --disable-mpi --enable-examples --enable-tests`
- `./ns3 build src/unified-bus/examples/ub-quick-example unified-bus-test`
- `build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/2nodes_single-tp`
- `build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/2nodes_single-tp --mtp-threads=2`

Expected:
- runtime startup is stable
- `MTP` build still works
- no regression in existing single-process path

### Validation on an MPI-enabled environment

Run:
- `./ns3 configure --enable-mpi --enable-mtp --enable-examples --enable-tests`
- `./ns3 build src/unified-bus/examples/ub-hybrid-smoke test-runner`
- `./ns3 run ub-hybrid-smoke --command-template "mpiexec -n 2 %s --test"`
- `./test.py -s mpi-example-ub-hybrid-smoke-2`

Expected:
- configure succeeds because MPI toolchain is present
- `ub-hybrid-smoke` builds
- two-rank smoke run exits cleanly
- MPI regression passes

## Risks to Watch During Implementation

- `UbTrafficGen` is a global singleton with mutable shared state; do not drag it into phase 1 hybrid smoke.
- `UbUtils` still contains process-global file and trace state; avoid using it in the hybrid smoke example unless absolutely required.
- `UbLink` currently assumes local scheduling via `ScheduleWithContext(...)`; do not overload it prematurely with remote semantics.
- Running two builds that both trigger CMake regeneration at the same time can race in this repository; keep configure/build validation serial while iterating.
- The current machine does not have MPI installed, so phase-2 through phase-5 runtime validation must be run on an MPI-enabled host.

## Completion Criteria

This plan is complete when all of the following are true:
- `ub-quick-example` starts reliably without requiring a pre-created `runlog` directory.
- `ub-quick-example` accepts an explicit case-path option and no longer depends on fragile argument ordering.
- `ub-hybrid-smoke` builds only when both `MPI` and `MTP` are enabled.
- `ub-hybrid-smoke` proves one minimal cross-rank `ub` flow on `2` ranks.
- MPI regression coverage exists for that smoke path.
- docs describe the new bring-up command and phase-1 limits accurately.
