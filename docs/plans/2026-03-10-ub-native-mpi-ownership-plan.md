# UB Native MPI Ownership Rules Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 收敛 unified-bus 在 native MPI / MTP+MPI 场景下的底层 ownership 与 remote/local 判定规则，并用 focused 单测锁住 rank 提取、builder 决策与配置装载阶段的 TP preload 调用边界。

**Architecture:** 继续保持 `UbPort` 统一、只让 `UbLink` / `UbRemoteLink` 区分本地与远端。把目前散落在 `ub-utils.cc` 的 MPI-rank 提取、本地 ownership 判定、remote link builder 判定抽成可复用且可测试的最小 helper；builder 和配置装载阶段的 TP preload 都统一走这些 helper，避免后续继续散落重复规则。`UbController::CreateTp(...)` 的语义保持不变：它只创建当前 controller 上的本地 TP，对端只作为 metadata 传入。测试优先覆盖纯规则与本地 builder 决策，不扩到更高层业务语义。

**Tech Stack:** ns-3 C++, unified-bus (`UbUtils`, `UbLink`, `UbRemoteLink`, `UbController`), MPI (`MpiInterface`), MTP packed `systemId`, unified-bus unit test suite.

---

## Preconditions

- 在当前 `feat-mpi` 分支继续做最小改动，不合主分支。
- 不在本计划里扩展性能对比或更大拓扑；只收敛底层规则与 focused tests。
- 不在本计划里改 `ub-hybrid-smoke` 的业务语义；它现在只作为已经通过的 smoke baseline。
- 未验证前不得宣称完成。

## Relevant Files To Read Before Coding

- `src/unified-bus/model/ub-utils.h`
- `src/unified-bus/model/ub-utils.cc`
- `src/unified-bus/model/ub-port.h`
- `src/unified-bus/model/ub-port.cc`
- `src/unified-bus/model/ub-remote-link.h`
- `src/unified-bus/model/ub-remote-link.cc`
- `src/unified-bus/test/ub-test.cc`
- `src/mpi/model/granted-time-window-mpi-interface.cc`
- `src/mpi/model/hybrid-simulator-impl.cc`
- `src/unified-bus/examples/ub-mpi-config-smoke.cc`

## Non-Goals For This Plan

- 不新增性能 benchmark。
- 不扩展 control frame 更复杂的系统级用例。
- 不在这轮把 `quick-example` 全量接成 MPI 正式入口。
- 不处理与当前目标无关的 warning 清理。

### Task 1: Extract ownership helpers into testable UB utility API

**Files:**
- Modify: `src/unified-bus/model/ub-utils.h`
- Modify: `src/unified-bus/model/ub-utils.cc`
- Test: `src/unified-bus/test/ub-test.cc`

**Step 1: Write the failing unit tests for pure ownership helpers**

Add focused tests for:
- extracting MPI rank from plain `systemId`
- extracting MPI rank from packed `systemId`
- checking whether two `systemId` values are on the same MPI rank

Expected failing state before implementation:
- helper functions are only anonymous-namespace internals in `ub-utils.cc`
- tests cannot call them directly

**Step 2: Introduce minimal public/static helper API in `UbUtils`**

Add the smallest possible API surface, for example:
```cpp
static uint32_t ExtractMpiRank(uint32_t systemId);
static bool IsSameMpiRank(uint32_t lhsSystemId, uint32_t rhsSystemId);
static bool IsNodeOwnedByCurrentRank(Ptr<Node> node);
```

Rules:
- under `NS3_MTP`, MPI rank is low 16 bits
- otherwise use full `systemId`
- no extra abstraction beyond what tests need

**Step 3: Move existing anonymous helper logic behind the new API**

Implementation requirements:
- keep behavior byte-for-byte equivalent to current successful smoke logic
- make old internal call sites delegate to the new helpers
- avoid duplicating rank logic in multiple places

**Step 4: Run focused unified-bus tests**

Run:
```bash
build/utils/ns3.44-test-runner-default --suite=unified-bus --verbose
```
Expected:
- new helper tests PASS
- existing unified-bus tests continue to PASS

### Task 2: Lock builder decision on MPI rank, not raw systemId

**Files:**
- Modify: `src/unified-bus/model/ub-utils.cc`
- Test: `src/unified-bus/test/ub-test.cc`

**Step 1: Write failing builder tests for packed-systemId edge cases**

Add tests that create nodes whose full `systemId` differs but MPI-rank portion is equal.

Cases:
- same rank, different high 16 bits -> builder must choose `UbLink`
- different rank -> builder must choose `UbRemoteLink`

Expected failing state before implementation:
- if any call site still compares raw `systemId`, same-rank packed ids could be misclassified

**Step 2: Make all builder decisions use the shared helper**

Specifically ensure `CreateUbChannelBetween(...)` uses only the shared helper logic, not open-coded comparisons.

**Step 3: Keep remote registration coupled to remote-link creation**

Rules:
- only remote links call `EnableMpiReceive()` on both ends
- local links must not enable MPI receive as a side effect

**Step 4: Run focused unified-bus tests again**

Run:
```bash
build/utils/ns3.44-test-runner-default --suite=unified-bus --verbose
```
Expected:
- packed-systemId builder tests PASS
- existing remote-link tests still PASS

### Task 3: Lock config-stage TP preload invocation to the shared ownership helper

**Files:**
- Modify: `src/unified-bus/model/ub-utils.cc`
- Test: `src/unified-bus/test/ub-test.cc`

**Step 1: Write failing tests for config preload invocation boundary**

Add focused tests for config-driven `CreateTp(...)` preload behavior:
- owner-side node preloads its local TP instance
- non-owner-side node does not perform a local preload call for that endpoint
- packed `systemId` case follows low-16-bit rank ownership

Testing guidance:
- prefer direct helper coverage when MPI runtime state is hard to fake
- keep assertions minimal and deterministic

**Step 2: Route config preload ownership through the shared helper**

Ensure `PreloadLocalTpIfOwned(...)` and any related path use `IsNodeOwnedByCurrentRank(...)` only.

**Step 3: Verify no incorrect cross-rank preload side effects**

Check that existing `CreateTp preloads TP instances from config` test still passes, and new ownership-specific tests cover the packed edge without changing `UbController::CreateTp(...)` semantics.

**Step 4: Run unified-bus suite**

Run:
```bash
build/utils/ns3.44-test-runner-default --suite=unified-bus --verbose
```
Expected:
- PASS

### Task 4: Run system-level regression checks on the already-working MPI baseline

**Files:**
- Verify only

**Step 1: Rebuild the touched targets**

Run:
```bash
cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target unified-bus-test ub-hybrid-smoke mpi-test
```
Expected:
- build succeeds

**Step 2: Run the focused MPI regression for removed interceptor mode**

Run:
```bash
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-hybrid-smoke-2-interceptor-removed --verbose
```
Expected:
- PASS

**Step 3: Re-run 2-rank TP smoke**

Run:
```bash
mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-hybrid-smoke-default --test --mode=tp --flow-size=1500 --stop-ms=50
```
Expected:
- PASS

**Step 4: Re-run 2-rank TP+CBFC smoke**

Run:
```bash
mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-hybrid-smoke-default --test --mode=tp --flow-control=cbfc --flow-size=1500 --stop-ms=50
```
Expected:
- PASS

### Task 5: Document conclusions and remaining gaps

**Files:**
- Modify: `docs/plans/2026-03-10-ub-native-mpi-ownership-plan.md`
- Optional notes only: `UNISON_README.md`

**Step 1: Append verification results to the plan**

Record:
- exact commands run
- whether helper tests passed
- whether MPI smoke stayed green

**Step 2: Record remaining known gaps**

Include only confirmed gaps, for example:
- quick-example / config-driven full MPI workflow not yet declared as formal native entrypoint
- larger-topology system tests still pending
- performance comparison matrix still pending

**Step 3: Stop after verification**

Do not merge branches or clean unrelated files in this plan.

---

## Phase 2: Config Native MPI Entry

### Task 6: Make config-driven MPI smoke a first-class regression entry

**Files:**
- Read: `src/unified-bus/examples/ub-mpi-config-smoke.cc`
- Modify: `src/mpi/test/mpi-test-suite.cc`
- Verify: `src/unified-bus/examples/ub-mpi-config-smoke.cc`

**Step 1: Freeze the current config smoke contract**

Record the intended contract in task notes:
- config path drives node/topology/tp construction
- MPI rank only drives which nodes/apps are locally active
- remote/local links are selected only by builder logic

**Step 2: Add a focused MPI regression entry for config-driven UB smoke**

Add an MPI example-as-test or focused system test that runs:
```bash
python3 ./ns3 run ub-mpi-config-smoke --no-build --command-template="mpiexec -n 2 %s --test <case-args>"
```

Expected:
- regression lives beside existing UB MPI smoke coverage
- no manual command is required to keep this route alive

**Step 3: Keep the test case minimal**

Rules:
- exactly 2 ranks
- fixed small topology
- one deterministic traffic source
- no performance assertions

**Step 4: Verify the new config regression**

Run the focused suite and confirm it passes.

### Task 7: Define MPI-mode config requirements explicitly

**Files:**
- Modify: `UNISON_README.md`
- Optional notes: `docs/plans/2026-03-10-ub-native-mpi-ownership-plan.md`

**Step 1: Document required config fields in MPI mode**

At minimum document:
- `node.csv` needs explicit `systemId`
- topology endpoints may cross rank
- TP config is a connection description, not an instruction to create remote-side objects on the current rank

**Step 2: Distinguish compatibility mode vs. strict MPI mode**

Document:
- single-process / legacy configs may omit `systemId` and default to `0`
- multi-process configs should be treated as explicit placement input

**Step 3: Keep docs minimal**

Do not expand into a full user guide; only record the rules the code enforces.

---

## Phase 3: Config Sanity Checks

### Task 8: Add fail-fast validation for invalid MPI config inputs

**Files:**
- Modify: `src/unified-bus/model/ub-utils.cc`
- Test: `src/unified-bus/test/ub-test.cc`

**Step 1: Write failing tests for invalid config inputs**

Add focused cases for:
- MPI-oriented config with missing or empty `systemId`
- malformed `systemId`
- topology rows referencing missing nodes
- TP rows referencing missing nodes or invalid endpoints

**Step 2: Add minimal validation at parse/build boundaries**

Rules:
- reject invalid inputs at the earliest deterministic point
- emit stable error messages
- do not continue with partially valid state

**Step 3: Keep validation scoped**

Do not add speculative schema validation; only cover fields required by current native MPI flow.

**Step 4: Run focused unified-bus tests**

Expected:
- invalid-input tests PASS
- existing tests stay green

### Task 9: Lock remote/local decision consistency for config builds

**Files:**
- Modify: `src/unified-bus/model/ub-utils.cc`
- Test: `src/unified-bus/test/ub-test.cc`

**Step 1: Add tests that exercise config-driven local and remote links together**

Cases:
- all-local topology
- one remote link
- multiple remote links

**Step 2: Assert builder decisions are consistent**

For each built link, assert:
- same-rank endpoints use `UbLink`
- cross-rank endpoints use `UbRemoteLink`
- only remote endpoints enable MPI receive

**Step 3: Verify ownership and builder checks do not drift apart**

Ensure config preload still follows the same helper logic used by link selection.

### Task 10: Add config preload boundary checks

**Files:**
- Modify: `src/unified-bus/test/ub-test.cc`
- Modify: `src/unified-bus/model/ub-utils.cc` if required

**Step 1: Add tests for mixed-rank TP config loading**

Cases:
- local endpoint gets preloaded on owner rank
- remote endpoint is not spuriously preloaded on non-owner rank
- packed `systemId` case follows low-16-bit ownership

**Step 2: Keep `UbController::CreateTp(...)` semantics unchanged**

Do not turn this API into a distributed creator; only constrain its config-driven caller.

**Step 3: Re-run unified-bus suite**

Expected:
- PASS

---

## Phase 4: Topology Matrix

### Task 11: Add a second config-driven topology smoke with more than one remote edge

**Files:**
- Create or modify case files under the existing config smoke case directory
- Modify: `src/unified-bus/examples/ub-mpi-config-smoke.cc` only if needed
- Modify: `src/mpi/test/mpi-test-suite.cc`

**Step 1: Define one additional deterministic topology**

Requirements:
- still only 2 ranks
- more than one remote link
- still small enough for quick regression

**Step 2: Reuse the same entrypoint**

Do not create a separate builder path; exercise the same config pipeline with a different case.

**Step 3: Add one focused regression**

Keep assertions minimal:
- process exits successfully
- deterministic `TEST ...` lines appear

### Task 12: Extend config smoke matrix to CBFC

**Files:**
- Modify case config as needed
- Modify regression registration as needed

**Step 1: Add one CBFC-enabled config smoke**

Rules:
- use small deterministic traffic
- validate functional success, not throughput

**Step 2: Keep control-frame expectations minimal**

Prefer asserting successful end-to-end completion plus stable smoke logs over fragile packet-count overfitting in this phase.

### Task 13: Add one slightly larger-flow regression

**Files:**
- Modify config case or smoke arguments
- Modify regression registration as needed

**Step 1: Keep this as a robustness check, not performance test**

Example intent:
- same topology
- larger flow than baseline
- still within quick/medium runtime

**Step 2: Avoid mixing thread-scaling claims into this phase**

Do not present this as benchmark evidence; it is only a stronger functional regression.

### Task 14: Re-run end-to-end regression set

**Files:**
- Verify only

**Step 1: Rebuild touched targets**

Run:
```bash
cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target unified-bus-test ub-hybrid-smoke mpi-test
```

**Step 2: Run unified-bus suite**

Run:
```bash
build/utils/ns3.44-test-runner-default --suite=unified-bus --verbose
```

**Step 3: Run focused MPI UB regressions**

Run the relevant suites for:
- interceptor-removed regression
- config MPI smoke baseline
- config MPI multi-remote regression
- config MPI CBFC regression

**Step 4: Re-run direct 2-rank smoke baselines**

Run:
```bash
mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-hybrid-smoke-default --test --mode=tp --flow-size=1500 --stop-ms=50
mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-hybrid-smoke-default --test --mode=tp --flow-control=cbfc --flow-size=1500 --stop-ms=50
```

**Step 5: Record remaining gaps**

Only record confirmed gaps, for example:
- quick-example still not formal MPI entrypoint
- performance comparison matrix still pending
- larger multi-rank topology matrix still pending

---

## Phase 1 Execution Log (2026-03-10)

### Code Changes Applied

- Added shared testable ownership helpers to `UbUtils`:
  - `ExtractMpiRank(uint32_t systemId)`
  - `IsSameMpiRank(uint32_t lhsSystemId, uint32_t rhsSystemId)`
  - `IsSystemOwnedByRank(uint32_t systemId, uint32_t currentRank)`
  - `IsNodeOwnedByCurrentRank(Ptr<Node> node)`
- Routed builder decision and config-stage preload ownership through the shared helpers.
- Added focused unit tests for:
  - packed/plain rank extraction
  - same-rank comparison on packed `systemId`
  - packed ownership-vs-rank rule
  - same-rank packed topology staying on `UbLink`
  - local-link path not enabling MPI receive

### Verification Commands Run

1. Focused helper regression after Task 1:
```bash
build/utils/ns3.44-test-runner-default --suite=unified-bus --verbose
```
Result: PASS

2. Builder packed-systemId regression after Task 2:
```bash
cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target unified-bus-test
build/utils/ns3.44-test-runner-default --suite=unified-bus --verbose
```
Result: PASS

3. Ownership helper regression after Task 3:
```bash
cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target unified-bus-test
build/utils/ns3.44-test-runner-default --suite=unified-bus --verbose
```
Result: PASS

4. Phase 1 system-level rebuild:
```bash
cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target unified-bus-test ub-hybrid-smoke mpi-test
```
Result: PASS

5. Focused MPI regression:
```bash
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-hybrid-smoke-2-interceptor-removed --verbose
```
Result: PASS

6. Direct 2-rank TP smoke:
```bash
mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-hybrid-smoke-default --test --mode=tp --flow-size=1500 --stop-ms=50
```
Result: PASS

7. Direct 2-rank TP + CBFC smoke:
```bash
mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-hybrid-smoke-default --test --mode=tp --flow-control=cbfc --flow-size=1500 --stop-ms=50
```
Result: PASS

### Confirmed Remaining Gaps

- `ub-mpi-config-smoke` still carries its own local `ExtractMpiRank(...)`; helper unification there is still pending.
- config-driven full native MPI entry flow is not yet declared as the formal default entrypoint.
- larger topology / multi-remote / performance comparison matrix is still pending beyond this phase.

## Phase 2 Execution Log (2026-03-10)

### Code Changes Applied

- Registered `ub-mpi-config-smoke` as a formal MPI regression entry in `src/mpi/test/mpi-test-suite.cc`.
- Added the reference log `src/mpi/test/mpi-example-ub-mpi-config-smoke-2.reflog`.
- Removed the example-local `ExtractMpiRank(...)` helper from `src/unified-bus/examples/ub-mpi-config-smoke.cc` and routed it through `UbUtils::ExtractMpiRank(...)`.
- Added a minimal MPI config contract note to `UNISON_README.md`:
  - `node.csv` owns placement via explicit `systemId`
  - `topology.csv` may span ranks; builder decides local vs remote link
  - `transport_channel.csv` describes TP connectivity metadata only

### Verification Commands Run

1. Phase 2 target rebuild:
```bash
cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target ub-mpi-config-smoke mpi-test
```
Result: PASS

2. Formal MPI regression suite:
```bash
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-smoke-2 --verbose
```
Result: PASS

3. Direct config-driven 2-rank smoke:
```bash
python3 ./ns3 run ub-mpi-config-smoke --no-build --command-template="mpiexec -n 2 %s --test --case-path=scratch/ub-mpi-minimal"
```
Result: PASS

### Confirmed Contract After Phase 2

- config path drives node / topology / routing / TP construction without introducing a separate remote TP creation contract.
- MPI rank only decides which locally owned nodes activate traffic tasks and preload local TP state.
- local vs remote link remains a builder decision derived from endpoint ownership, not from CSV link annotations.

### Confirmed Remaining Gaps

- current MPI-only smoke oracle is still sender-heavy: ranks with `localTaskCount == 0` are treated as locally passed, so receive-side regressions can still be under-observed on that path.
- the new config-driven `MTP+MPI` regression is still a minimal 4-node case; larger topology / multi-remote / control-frame coverage remains pending.

## Phase 2A Ownership Regression Log (2026-03-10)

### Code Changes Applied

- Added a new MPI regression entry `mpi-example-ub-mpi-config-tp-ownership-2` that reuses `ub-mpi-config-smoke` with `--verify-tp-ownership`.
- Extended `ub-mpi-config-smoke` with a minimal `--verify-tp-ownership` mode.
- The ownership check walks each local `UbController` view and asserts:
  - nodes owned by the current MPI rank have their config-preloaded local TP instance
  - nodes not owned by the current MPI rank do not have that local TP instance precreated

### Verification Commands Run

1. Red test after adding the new suite before implementation:
```bash
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-tp-ownership-2 --verbose
```
Result: FAIL as expected (no ownership-check mode implemented yet)

2. Rebuild after implementation:
```bash
cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target ub-mpi-config-smoke mpi-test
```
Result: PASS

3. New ownership regression:
```bash
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-tp-ownership-2 --verbose
```
Result: PASS

4. Direct 2-rank ownership smoke:
```bash
python3 ./ns3 run ub-mpi-config-smoke --no-build --command-template="mpiexec -n 2 %s --test --case-path=scratch/ub-mpi-minimal --verify-tp-ownership"
```
Result: PASS

5. Backward-compat regression of the original config smoke:
```bash
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-smoke-2 --verbose
```
Result: PASS

### Confirmed Outcome

- config-stage TP preload ownership is now locked by a real 2-rank MPI regression rather than only by single-process unit coverage.
- a future regression that recreates remote-side placeholder TP instances on the wrong rank should now fail this dedicated suite.

## Phase 2B Config Hybrid Regression Log (2026-03-10)

### Code Changes Applied

- Extended `ub-mpi-config-smoke` to accept `--mtp-threads > 1` and enter the `MTP+MPI` hybrid path through `MtpInterface::Enable(...)`.
- Added `--verify-packed-systemid` to assert that nodes initially owned by the current MPI rank are repacked at runtime into hybrid `localSystemId << 16 | mpiRank` form.
- Added a new config case `scratch/ub-mpi-hybrid-minimal`:
  - 4 nodes total
  - 2 nodes per MPI rank
  - local links on each rank and one remote inter-rank link
  - bidirectional small flows so both ranks execute tasks
- Registered the formal regression entry `mpi-example-ub-mpi-config-hybrid-smoke-2`.

### Verification Commands Run

1. Red test before implementation:
```bash
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-smoke-2 --verbose
```
Result: FAIL as expected

2. Rebuild after implementation:
```bash
cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target ub-mpi-config-smoke mpi-test
```
Result: PASS

3. Formal hybrid regression:
```bash
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-smoke-2 --verbose
```
Result: PASS

4. Direct config-driven hybrid smoke:
```bash
python3 ./ns3 run ub-mpi-config-smoke --no-build --command-template="mpiexec -n 2 %s --test --case-path=scratch/ub-mpi-hybrid-minimal --mtp-threads=2 --verify-packed-systemid --verify-tp-ownership --stop-ms=50"
```
Result: PASS

5. Backward-compat regressions after hybrid support:
```bash
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-smoke-2 --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-tp-ownership-2 --verbose
```
Result: PASS

### Confirmed Outcome

- config-driven unified-bus now has a verified native `MTP+MPI` smoke entry instead of an explicit hybrid rejection path.
- runtime packed `systemId` behavior is now checked in a real hybrid run rather than only inferred from helper logic and MPI internals.

## Phase 2C Config Hybrid CBFC Regression Log (2026-03-10)

### Code Changes Applied

- Extended `ub-mpi-config-smoke` with `--verify-cbfc-control` so the config-driven hybrid path can assert cross-rank `CBFC` control traffic instead of only raw payload delivery.
- Added boundary-port observation on `UbPort::TraComEventNotify` for ports with `HasMpiReceive() == true`, then decoded `UbDatalinkHeader` / `UbDatalinkControlCreditHeader` directly from transmitted packets.
- Loaded the expected `CBFC` priority from `transport_channel.csv` so the regression checks the configured virtual lane, not a hard-coded default.
- Strengthened the hybrid `CBFC` regression with `--verify-cbfc-control-count`, which counts boundary data/ack packets and derives the expected control-frame count from actual on-wire packet sizes at the MPI boundary.
- Added a new config case `scratch/ub-mpi-hybrid-cbfc-minimal` and the formal MPI regression entry `mpi-example-ub-mpi-config-hybrid-cbfc-2`.

### Verification Commands Run

1. Red test after strengthening the suite contract:
```bash
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-cbfc-2 --verbose
```
Result: FAIL as expected before `--verify-cbfc-control-count` was implemented

2. Direct hybrid `CBFC` debug run during bring-up:
```bash
python3 ./ns3 run ub-mpi-config-smoke --no-build --command-template="mpiexec -n 2 %s --test --case-path=scratch/ub-mpi-hybrid-cbfc-minimal --mtp-threads=2 --verify-packed-systemid --verify-tp-ownership --verify-cbfc-control --verify-cbfc-control-count --stop-ms=50"
```
Observed intermediate failures:
- no TP-packet count source when trying to rely on `TpRecvNotify`
- count mismatch when using payload-only size instead of boundary on-wire size

3. Formal hybrid `CBFC` regression after fixing the count oracle:
```bash
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-cbfc-2 --verbose
```
Result: PASS

4. Direct config-driven hybrid `CBFC` smoke:
```bash
python3 ./ns3 run ub-mpi-config-smoke --no-build --command-template="mpiexec -n 2 %s --test --case-path=scratch/ub-mpi-hybrid-cbfc-minimal --mtp-threads=2 --verify-packed-systemid --verify-tp-ownership --verify-cbfc-control --verify-cbfc-control-count --stop-ms=50"
```
Result: PASS

5. Backward-compat regressions after `CBFC` support:
```bash
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-smoke-2 --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-tp-ownership-2 --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-smoke-2 --verbose
```
Result: PASS

### Confirmed Outcome

- config-driven native `MTP+MPI` support now has a formal `CBFC` regression, not only a payload-only hybrid smoke.
- [superseded by Phase 3] the first revision attached the `CBFC` check at the MPI boundary transmit path; this was useful for bring-up but remained a synthetic oracle.
- [superseded by Phase 3] the `--verify-cbfc-control-count` count oracle was an intermediate bring-up tool and is no longer treated as the correct long-term contract.

### Remaining Confirmed Gaps

- this regression uses a minimal 2-rank topology and only proves one remote boundary path at a time.
- it does not yet measure performance or compare single-process / multi-thread / multi-process throughput and latency.
- it does not yet stress multiple simultaneous remote links or stronger `CBFC` corner cases such as multiple priorities returning credits in the same control frame.

## Phase 2D Smoke Oracle Cleanup Log (2026-03-10)

### Code Changes Applied

- Removed the `ub-hybrid-smoke` remote-link packet oracle that decoded `Packet` bytes on `UbRemoteLink` and asserted `TP data / ACK` semantics at the link boundary.
- Kept `ub-hybrid-smoke` on the already-existing end-to-end `UbTransportChannel::TpRecvNotify` and task-complete signals only.
- Added repository-level test guidance to `AGENTS.md`:
  - `UbLink` / `UbRemoteLink` only carry serialized `Packet` bytes
  - semantic interpretation belongs to `UbSwitch`, flow-control, and transport handling paths
  - when a test must parse packets, it should reuse or strictly mirror unified-bus parsing boundaries rather than inventing an extra oracle
- Restored and kept the formal hybrid `CBFC` config regression entry `mpi-example-ub-mpi-config-hybrid-cbfc-2`.

### Verification Commands Run

1. Rebuild after removing the link-level oracle:
```bash
cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target ub-hybrid-smoke ub-mpi-config-smoke mpi-test
```
Result: PASS

2. Direct TP smoke after oracle cleanup:
```bash
python3 ./ns3 run ub-hybrid-smoke --no-build --command-template='mpiexec -n 2 %s --test --mode=tp --flow-size=1500 --stop-ms=50'
```
Result: PASS

3. Direct TP + `CBFC` smoke after oracle cleanup:
```bash
python3 ./ns3 run ub-hybrid-smoke --no-build --command-template='mpiexec -n 2 %s --test --mode=tp --flow-control=cbfc --flow-size=1500 --stop-ms=50'
```
Result: PASS

4. Focused regressions after oracle cleanup:
```bash
build/utils/ns3.44-test-runner-default --suite=unified-bus --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-hybrid-smoke-2 --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-smoke-2 --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-smoke-2 --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-cbfc-2 --verbose
```
Result: PASS

### Confirmed Outcome

- `ub-hybrid-smoke` no longer treats `UbRemoteLink` as a protocol-semantics observation point.
- the current MPI-native smoke baseline is now aligned with the repository rule that link only transports serialized `Packet` bytes and higher-level meaning is established inside unified-bus processing paths.
- the remaining questionable oracle is no longer in `ub-hybrid-smoke`; it is now concentrated in `ub-mpi-config-smoke`, which makes the next cleanup step well scoped.

### Updated Confirmed Gaps

- `ub-mpi-config-smoke` still needs its `CBFC` observation point moved from boundary-port transmit into the real flow-control receive/restore path.
- current `CBFC` config validation is still tied to a single remote boundary path and does not yet prove multi-priority credit returns in the same control frame.
- `LDST` has not yet been proven in a dedicated 2-rank native MPI regression, even though the current link/builder model is intended to be protocol-agnostic at the serialized-packet level.

---

## Phase 3: Config Oracle Convergence

### Task 15: Re-audit `ub-mpi-config-smoke` against the link-only transport rule

**Files:**
- Read: `src/unified-bus/examples/ub-mpi-config-smoke.cc`
- Read: `src/unified-bus/model/ub-switch.cc`
- Read: `src/unified-bus/model/protocol/ub-datalink.cc`
- Read: `src/unified-bus/model/protocol/ub-flow-control.cc`
- Modify: `docs/plans/2026-03-10-ub-native-mpi-ownership-plan.md`

**Step 1: Freeze the audit rules in task notes**

Record these rules before any code changes:
- `UbLink` / `UbRemoteLink` only move serialized packets
- packet meaning is established by unified-bus processing logic, not by the link
- test-side parsing must reuse or strictly mirror the production parsing boundary

**Step 2: Inventory every assumption inside `ObserveBoundaryPortTransmit(...)`**

Explicitly classify each assumption as:
- raw framing check
- mirrored production parsing
- extra oracle assumption that should be removed

**Step 3: Append the audit results to this plan**

Do not modify production code in this task; only freeze what is actually being assumed today.

### Task 16: Shrink or replace the boundary-port `CBFC` oracle

**Files:**
- Modify: `src/unified-bus/examples/ub-mpi-config-smoke.cc`
- Read: `src/unified-bus/model/ub-switch.h`
- Read: `src/unified-bus/model/ub-switch.cc`
- Read: `src/unified-bus/model/protocol/ub-datalink.h`
- Read: `src/unified-bus/model/protocol/ub-datalink.cc`

**Step 1: Write the failing expectation**

Desired end state:
- no assertion depends on “link saw a packet, therefore it semantically belongs to X”
- control-frame checks only assert facts that the real production parsing path also establishes

**Step 2: Prefer reuse or strict mirroring of production parsing**

Allowed directions:
- reuse helpers such as `UbDataLink::ParseCreditHeader(...)` where practical
- mirror `UbSwitch::GetPacketType(...)` packet-type boundaries exactly

Disallowed directions:
- inventing a simplified single-VL control-frame oracle
- deriving extra semantics at the link boundary that `UbSwitch` / flow-control never use

**Step 3: If boundary-port parsing remains too synthetic, add the smallest possible formal trace**

Only if needed, add one minimal production trace point closer to:
- `UbSwitch::SinkControlFrame(...)`, or
- flow-control receive/restore handling

Rules:
- smallest possible surface
- no new behavior
- trace only, not logic refactor

**Step 4: Re-run focused config regressions**

Run:
```bash
cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target ub-mpi-config-smoke mpi-test
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-smoke-2 --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-smoke-2 --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-cbfc-2 --verbose
```
Expected:
- PASS

### Task 17: Lock the cleaned config oracle with direct smoke

**Files:**
- Verify only

**Step 1: Re-run direct config hybrid smoke**

Run:
```bash
python3 ./ns3 run ub-mpi-config-smoke --no-build --command-template="mpiexec -n 2 %s --test --case-path=scratch/ub-mpi-hybrid-minimal --mtp-threads=2 --verify-packed-systemid --verify-tp-ownership --stop-ms=50"
```
Expected:
- PASS

**Step 2: Re-run direct config hybrid `CBFC` smoke**

Run:
```bash
python3 ./ns3 run ub-mpi-config-smoke --no-build --command-template="mpiexec -n 2 %s --test --case-path=scratch/ub-mpi-hybrid-cbfc-minimal --mtp-threads=2 --verify-packed-systemid --verify-tp-ownership --verify-cbfc-control --stop-ms=50"
```
Expected:
- PASS

---

## Phase 3 Execution Log (2026-03-10)

### Audit Result

- [fact] `ObserveBoundaryPortTransmit(...)` had three kinds of assumptions mixed together:
  - raw framing inspection on `TraComEventNotify`
  - partial mirroring of production header parsing
  - extra oracle assumptions that production never uses: exact control-frame count from boundary packet sizes, and link-boundary semantic attribution
- [fact] unified-bus production semantics for `CBFC` are established in flow-control receive/restore handling, not in `UbLink` / `UbRemoteLink` and not in boundary-port transmit callbacks.
- [inference] keeping the count oracle would continue to couple the regression to a synthetic observation layer, even if the current minimal case still passes.

### Code Changes Applied

- Added a minimal trace source `ControlCreditRestoreNotify` on `UbCbfc`, emitted from the real `CbfcRestoreCrd(...)` / `CbfcSharedRestoreCrd(...)` receive path.
- Replaced `ub-mpi-config-smoke` boundary-port packet decoding with flow-control restore observation on MPI boundary ports.
- Removed `--verify-cbfc-control-count` and its packet-size/count oracle from `ub-mpi-config-smoke`.
- Updated `mpi-example-ub-mpi-config-hybrid-cbfc-2` to use the cleaned `--verify-cbfc-control` contract only.

### Verification Commands Run

1. Rebuild after moving the oracle:
```bash
cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target ub-mpi-config-smoke mpi-test
```
Result: PASS

2. Focused config regressions:
```bash
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-smoke-2 --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-smoke-2 --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-cbfc-2 --verbose
```
Result: PASS

3. Direct hybrid smoke:
```bash
python3 ./ns3 run ub-mpi-config-smoke --no-build --command-template="mpiexec -n 2 %s --test --case-path=scratch/ub-mpi-hybrid-minimal --mtp-threads=2 --verify-packed-systemid --verify-tp-ownership --stop-ms=50"
```
Result: PASS

4. Direct hybrid `CBFC` smoke:
```bash
python3 ./ns3 run ub-mpi-config-smoke --no-build --command-template="mpiexec -n 2 %s --test --case-path=scratch/ub-mpi-hybrid-cbfc-minimal --mtp-threads=2 --verify-packed-systemid --verify-tp-ownership --verify-cbfc-control --stop-ms=50"
```
Result: PASS

### Confirmed Outcome

- `ub-mpi-config-smoke` no longer derives `CBFC` semantics by decoding serialized packets at the link/boundary layer.
- `CBFC` verification now anchors at the real flow-control restore path, which is the production boundary that establishes returned-credit meaning.
- the known-wrong count oracle is removed rather than patched.

### Remaining Confirmed Gaps

- the current `CBFC` config regression still proves only one remote boundary path in a minimal 2-rank topology.
- stronger multi-priority / multi-flow `CBFC` coverage is still pending.
- `LDST` still needs an explicit 2-rank native MPI proof.

---

## Phase 4: Protocol-Agnostic Proof

### Task 18: Prove the same remote-link path works for `LDST`

**Files:**
- Read: `src/unified-bus/model/protocol/ub-ldst-api.cc`
- Read: `src/unified-bus/model/ub-ldst-thread.cc`
- Read: `src/unified-bus/model/ub-switch.cc`
- Create or modify: a minimal 2-rank case under `scratch/`
- Modify: `src/unified-bus/examples/ub-mpi-config-smoke.cc` only if the current config path can already drive `LDST`
- Or create: a dedicated minimal `LDST` MPI smoke if the config entrypoint cannot express `LDST` cleanly

**Step 1: Freeze the hypothesis**

Record the hypothesis explicitly:
- `UbLink` / `UbRemoteLink` are protocol-agnostic carriers of serialized packets
- therefore `LDST` should not need link-layer special handling beyond builder/ownership already in place

**Step 2: Write the smallest failing 2-rank `LDST` regression**

Requirements:
- exactly 2 ranks
- one remote boundary path
- deterministic success signal
- no performance assertion

**Step 3: Prefer proving the current path over adding new abstractions**

Priority order:
1. reuse `ub-mpi-config-smoke` if it can naturally drive `LDST`
2. otherwise add one dedicated minimal smoke for `LDST`

Do not refactor link/model code unless the proof actually fails.

**Step 4: Investigate failures only at the true missing layer**

If red:
- first determine whether the gap is in config expressiveness
- then ownership/object preload
- only last consider protocol-specific model issues

**Step 5: Verify the new `LDST` regression**

Run the focused direct smoke and, if added, the dedicated suite.

### Task 19: Add one multi-remote-edge proof on the same builder path

**Files:**
- Create or modify case files under `scratch/`
- Modify: `src/mpi/test/mpi-test-suite.cc`
- Modify: relevant `.reflog` files as needed

**Step 1: Define one deterministic 2-rank topology with more than one remote edge**

Requirements:
- still small
- same config-driven builder path
- deterministic task completion

**Step 2: Keep assertions minimal**

Assert only:
- process exits successfully
- stable `TEST ...` lines
- no builder misclassification regressions

**Step 3: Run the focused MPI suite**

Expected:
- PASS

---

## Phase 4 Execution Log (2026-03-10)

### Code Changes Applied

- Added a new minimal 2-rank `LDST` config case at `scratch/ub-mpi-hybrid-ldst-minimal/`.
- Reused `ub-mpi-config-smoke` unchanged to prove `MEM_STORE` traffic crosses the same remote-link / builder path under `MTP+MPI`.
- Added a new deterministic 2-rank multi-remote case at `scratch/ub-mpi-hybrid-multi-remote/` with two disjoint cross-rank edges.
- Registered two formal MPI regressions:
  - `mpi-example-ub-mpi-config-hybrid-ldst-2`
  - `mpi-example-ub-mpi-config-hybrid-multi-remote-2`

### Verification Commands Run

1. Direct `LDST` proof:
```bash
python3 ./ns3 run ub-mpi-config-smoke --no-build --command-template="mpiexec -n 2 %s --test --case-path=scratch/ub-mpi-hybrid-ldst-minimal --mtp-threads=2 --verify-packed-systemid --stop-ms=50"
```
Result: PASS

2. Direct multi-remote proof:
```bash
python3 ./ns3 run ub-mpi-config-smoke --no-build --command-template="mpiexec -n 2 %s --test --case-path=scratch/ub-mpi-hybrid-multi-remote --mtp-threads=2 --verify-packed-systemid --verify-tp-ownership --stop-ms=50"
```
Result: PASS

3. Rebuild after adding formal suites:
```bash
cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target mpi-test
```
Result: PASS

4. Formal `LDST` regression:
```bash
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-ldst-2 --verbose
```
Result: PASS

5. Formal multi-remote regression:
```bash
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-multi-remote-2 --verbose
```
Result: PASS

### Confirmed Outcome

- the current remote-link / ownership / builder path is now proven for `LDST` without adding protocol-specific link code.
- config-driven `MTP+MPI` now has a formal multi-remote regression instead of only a single-cross-rank-edge minimal case.

### Remaining Confirmed Gaps

- stronger `CBFC` coverage with more than one active flow or priority is still pending.
- larger topology / performance comparison work is still pending outside this proof phase.

---

## Phase 5: Final Checkpoint

### Task 20: Run the final focused verification set

**Files:**
- Verify only

**Step 1: Rebuild touched targets**

Run:
```bash
cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target unified-bus-test ub-hybrid-smoke ub-mpi-config-smoke mpi-test
```
Expected:
- PASS

**Step 2: Run the final regression subset**

Run:
```bash
build/utils/ns3.44-test-runner-default --suite=unified-bus --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-hybrid-smoke-2 --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-hybrid-smoke-2-interceptor-removed --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-smoke-2 --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-smoke-2 --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-cbfc-2 --verbose
```
Expected:
- PASS

**Step 3: Run the direct smoke baselines**

Run:
```bash
python3 ./ns3 run ub-hybrid-smoke --no-build --command-template='mpiexec -n 2 %s --test --mode=tp --flow-size=1500 --stop-ms=50'
python3 ./ns3 run ub-hybrid-smoke --no-build --command-template='mpiexec -n 2 %s --test --mode=tp --flow-control=cbfc --flow-size=1500 --stop-ms=50'
python3 ./ns3 run ub-mpi-config-smoke --no-build --command-template="mpiexec -n 2 %s --test --case-path=scratch/ub-mpi-hybrid-minimal --mtp-threads=2 --verify-packed-systemid --verify-tp-ownership --stop-ms=50"
python3 ./ns3 run ub-mpi-config-smoke --no-build --command-template="mpiexec -n 2 %s --test --case-path=scratch/ub-mpi-hybrid-cbfc-minimal --mtp-threads=2 --verify-packed-systemid --verify-tp-ownership --verify-cbfc-control --stop-ms=50"
```
Expected:
- PASS

### Task 21: Write the final handoff report and stop

**Files:**
- Modify: `docs/plans/2026-03-10-ub-native-mpi-ownership-plan.md`
- Optional: a short report under `docs/workflows/` or another agreed notes path

**Step 1: Record exactly what is proven**

At minimum list:
- builder/ownership rules
- config-driven native MPI baseline
- hybrid `TP`
- hybrid `CBFC`
- whether `LDST` proof succeeded or remains pending

**Step 2: Record confirmed remaining gaps only**

Examples:
- performance comparison matrix still pending
- larger topology matrix still pending
- stronger multi-priority `CBFC` coverage still pending

**Step 3: Stop after report**

Do not merge to main and do not clean unrelated untracked files in this checkpoint.

---

## Phase 6: Main-Branch Readiness

### Task 22: Add a formal 2-rank `LDST` regression

**Files:**
- Read: `src/unified-bus/model/protocol/ub-ldst-api.cc`
- Read: `src/unified-bus/model/ub-ldst-thread.cc`
- Read: `src/unified-bus/model/ub-switch.cc`
- Create or modify: minimal `LDST` case files under `scratch/`
- Modify: `src/mpi/test/mpi-test-suite.cc`
- Create: matching `.reflog` file if needed

**Step 1: Freeze the proof target**

Record the intended proof:
- `UbLink` / `UbRemoteLink` are serialized-packet carriers only
- therefore `LDST` should not require link-layer special handling

**Step 2: Write the smallest failing 2-rank regression**

Requirements:
- exactly 2 ranks
- one remote boundary path
- deterministic completion signal
- no performance assertion

**Step 3: Reuse existing entrypoints before adding new ones**

Priority:
1. reuse `ub-mpi-config-smoke` if it can naturally express the `LDST` path
2. otherwise add one dedicated minimal `LDST` MPI smoke

**Step 4: Verify the new regression**

Run the direct smoke and the formal suite.

Expected:
- PASS, or
- a precise recorded blocker at config / ownership / endpoint / protocol level

### Task 23: Add one multi-remote-edge config regression

**Files:**
- Create or modify: case files under `scratch/`
- Modify: `src/mpi/test/mpi-test-suite.cc`
- Create or modify: matching `.reflog`

**Step 1: Define one deterministic small topology**

Requirements:
- still only 2 ranks
- more than one remote edge
- same config-driven builder path
- deterministic end-to-end completion

**Step 2: Keep assertions minimal**

Assert only:
- process exits successfully
- stable `TEST ...` lines appear
- no builder misclassification regression

**Step 3: Verify the regression**

Run the direct smoke and the formal suite.

Expected:
- PASS

### Task 24: Add one stronger `CBFC` regression with more than one active priority or flow

**Files:**
- Create or modify: `scratch/` case files
- Modify: `src/unified-bus/examples/ub-mpi-config-smoke.cc` only if required
- Modify: `src/mpi/test/mpi-test-suite.cc`
- Create or modify: matching `.reflog`

**Step 1: Define the smallest stronger `CBFC` case**

Requirements:
- still deterministic
- at least two active flows or priorities
- keeps runtime in quick/medium range

**Step 2: Avoid overfitting the oracle**

Rules:
- prefer success and confirmed returned-credit facts over fragile exact packet-order assumptions
- do not reintroduce single-VL control-frame assumptions

**Step 3: Verify the stronger `CBFC` regression**

Expected:
- PASS

### Task 25: Run merge-candidate review and cleanup

**Files:**
- Review: all touched files
- Modify: only if review finds correctness or maintainability issues directly relevant to this feature

**Step 1: Do a focused code review**

Review for:
- hidden link-level semantic assumptions
- duplicated ownership / rank logic
- regression fragility
- accidental scope creep

**Step 2: Apply only direct fixes**

Rules:
- no unrelated cleanup
- no warning-only cleanup
- no new abstraction unless it removes a proven duplication or risk in this feature path

**Step 3: Re-run the focused regression set**

Run:
```bash
cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target unified-bus-test ub-hybrid-smoke ub-mpi-config-smoke mpi-test
build/utils/ns3.44-test-runner-default --suite=unified-bus --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-hybrid-smoke-2 --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-hybrid-smoke-2-interceptor-removed --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-smoke-2 --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-smoke-2 --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-cbfc-2 --verbose
```
Plus all new `LDST` / multi-remote / stronger-`CBFC` suites added in this phase.

### Task 26: Write the merge-readiness report

**Files:**
- Modify: `docs/plans/2026-03-10-ub-native-mpi-ownership-plan.md`
- Optional: short report under `docs/workflows/` or another agreed notes path

**Step 1: Record what is now proven**

At minimum:
- ownership and builder rules
- config-driven native MPI baseline
- hybrid `TP`
- hybrid `CBFC`
- `LDST` 2-rank status
- multi-remote-edge status

**Step 2: Give a binary recommendation**

Choose one:
- `merge-ready`
- `not yet merge-ready`

If not ready, list only the concrete blockers.

---

## Phase 7: Confidence Expansion

### Task 27: Add a larger topology matrix on the same config path

**Files:**
- Create or modify: additional `scratch/` cases
- Modify: `src/mpi/test/mpi-test-suite.cc`
- Create or modify: matching `.reflog`

**Step 1: Add one larger but still deterministic topology**

Requirements:
- more nodes than the current minimal cases
- still only as large as needed to prove builder / ownership stability
- same config-driven path

**Step 2: Add one focused regression per topology, not a matrix explosion**

Rules:
- keep coverage additive
- avoid multiplying cases unless each adds a distinct failure mode

### Task 28: Add a simple performance baseline

**Files:**
- Modify or create: benchmark notes / scripts only if needed
- Modify: report docs only

**Step 1: Define a non-ambitious baseline**

Compare only a small set such as:
- single-process / single-thread
- single-process / multi-thread
- multi-process or multi-process + multi-thread

**Step 2: Keep this informational, not gating**

Rules:
- do not fail regressions on throughput numbers
- only record measurements and obvious anomalies

### Task 29: Add one stronger robustness check

**Files:**
- Create or modify: one additional case or smoke invocation
- Modify docs if needed

**Step 1: Pick one robustness axis**

Examples:
- slightly larger flow
- multiple simultaneous flows
- slightly longer run duration

**Step 2: Keep it bounded**

This is not a soak test; it is only a stronger confidence check.

### Task 30: Write the confidence-expansion summary

**Files:**
- Modify: `docs/plans/2026-03-10-ub-native-mpi-ownership-plan.md`

**Step 1: Record what the larger coverage adds**

At minimum:
- which new failure modes are now covered
- whether any performance anomaly was observed
- what still remains outside scope

**Step 2: Stop after summary**

Do not turn this phase into open-ended coverage expansion.

---

## End Condition For This Round

This round is complete when all of the following are true:

- `ub-hybrid-smoke` stays on end-to-end TP semantics only
- `ub-mpi-config-smoke` no longer carries a known-wrong link/boundary oracle
- config-driven native MPI baseline and hybrid `CBFC` regressions are green
- either a minimal 2-rank `LDST` proof is green, or a precise reason is recorded for why it is still pending
- the final plan log records exact verification commands and remaining confirmed gaps
