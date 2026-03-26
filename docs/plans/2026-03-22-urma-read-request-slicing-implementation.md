# URMA Read Request Slicing Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Make `URMA_READ` follow spec-aligned request slicing semantics: each read request slice carries zero payload but non-zero logical length, each slice generates exactly one `READ_RESPONSE`, and the WQE completes only after all slice responses arrive.

**Architecture:** Keep the existing `Jetty -> TA -> TP` layering and reuse `m_tpRelatedRemoteRequests` as the only target-side response injection point. Introduce explicit `logicalBytes` / `payloadBytes` / `carrierBytes` semantics in `UbWqeSegment`, then update TP send/receive code so `READ` request slices are zero-payload single-packet requests while `READ_RESPONSE` remains TP-fragmentable.

**Tech Stack:** ns-3 C++20, unified-bus module, existing unit tests in `src/unified-bus/test/ub-test.cc`, Ninja/test-runner build flow

---

### Task 1: Add failing metadata tests for logical/payload/carrier byte semantics

**Files:**
- Modify: `src/unified-bus/test/ub-test.cc`
- Test: `src/unified-bus/test/ub-test.cc`

**Step 1: Write the failing test**

Add assertions in the URMA read metadata propagation test so a `READ` request WQE and its first slice must satisfy:

```cpp
NS_TEST_ASSERT_MSG_EQ(wqe->GetLogicalBytes(), payloadBytes, "READ WQE logicalBytes should equal request bytes");
NS_TEST_ASSERT_MSG_EQ(wqe->GetPayloadBytes(), 0u, "READ WQE payloadBytes should be zero");
NS_TEST_ASSERT_MSG_EQ(wqe->GetCarrierBytes(), 1u, "READ WQE carrierBytes should force one request packet");

NS_TEST_ASSERT_MSG_EQ(segment->GetLogicalBytes(), payloadBytes, "READ request slice logicalBytes should preserve request bytes");
NS_TEST_ASSERT_MSG_EQ(segment->GetPayloadBytes(), 0u, "READ request slice payloadBytes should be zero");
NS_TEST_ASSERT_MSG_EQ(segment->GetCarrierBytes(), 1u, "READ request slice carrierBytes should be one");
```

**Step 2: Run test to verify it fails**

Run:

```bash
ninja -C cmake-cache unified-bus-test -j 1
./build/utils/ns3.44-test-runner-default --test-name=unified-bus
```

Expected: build or test failure because the new getters/fields do not exist yet.

**Step 3: Write minimal implementation**

Add the new fields/getters/setters on `UbWqe` and `UbWqeSegment`, and initialize them for read/write defaults.

**Step 4: Run test to verify it passes**

Run the same commands.

Expected: metadata assertions pass, later tests may still fail.

**Step 5: Commit**

```bash
git add src/unified-bus/model/ub-datatype.h src/unified-bus/model/protocol/ub-function.cc src/unified-bus/test/ub-test.cc
git commit -m "test(urma): cover read logical and payload byte metadata"
```

### Task 2: Make TP send path distinguish logical bytes from payload bytes

**Files:**
- Modify: `src/unified-bus/model/ub-datatype.h`
- Modify: `src/unified-bus/model/protocol/ub-transport.cc`
- Modify: `src/unified-bus/model/protocol/ub-transport.h`
- Test: `src/unified-bus/test/ub-test.cc`

**Step 1: Write the failing test**

Add a new regression test that creates a local static TP pair and a `128 * 1024` `URMA_READ`, then observes target-side packet receives through `TpRecvNotify` with packet tracing enabled. Assert:

```cpp
NS_TEST_ASSERT_MSG_EQ(m_readRequestPacketCount, 2u, "128KiB read should produce 2 read request packets");
NS_TEST_ASSERT_MSG_EQ(m_zeroPayloadReadRequestCount, 2u, "each read request packet should carry zero payload");
```

Use `GlobalValue::Bind("UB_RECORD_PKT_TRACE", BooleanValue(true));` before creating the topology and count only initiator->target request packets.

**Step 2: Run test to verify it fails**

Run:

```bash
ninja -C cmake-cache unified-bus-test -j 1
./build/utils/ns3.44-test-runner-default --test-name=unified-bus
```

Expected: failure because `READ` request packets still carry non-zero payload bytes.

**Step 3: Write minimal implementation**

Update TP send semantics:

- use `payloadBytes` when creating packets and when writing `MAETAH.Length` choose `logicalBytes`;
- use `carrierBytes` for send progress and PSN accounting;
- keep `READ` request and `TAACK` on one packet by setting `carrierBytes = 1`.

Representative logic:

```cpp
uint32_t progressBytes = currentSegment->GetCarrierBytesLeftThisPacket();
uint32_t payloadBytes = currentSegment->GetPayloadBytesLeftThisPacket();
Ptr<Packet> p = GenDataPacket(currentSegment, payloadBytes);
currentSegment->UpdateSentBytes(progressBytes);
```

**Step 4: Run test to verify it passes**

Run the same commands.

Expected: request packet count and zero-payload assertions pass.

**Step 5: Commit**

```bash
git add src/unified-bus/model/ub-datatype.h src/unified-bus/model/protocol/ub-transport.cc src/unified-bus/model/protocol/ub-transport.h src/unified-bus/test/ub-test.cc
git commit -m "fix(urma): send read request slices without payload"
```

### Task 3: Make TP receive path reassemble by payload bytes while preserving logical length

**Files:**
- Modify: `src/unified-bus/model/protocol/ub-transport.cc`
- Modify: `src/unified-bus/model/ub-datatype.h`
- Test: `src/unified-bus/test/ub-test.cc`

**Step 1: Write the failing test**

Extend the new multi-slice regression so target-side TA execution sees exactly two completed read request slices and initiator-side sees exactly two completed read responses:

```cpp
NS_TEST_ASSERT_MSG_EQ(m_targetReadRequestSliceCount, 2u, "128KiB read should complete 2 request slices at TA");
NS_TEST_ASSERT_MSG_EQ(m_readResponseCount, 2u, "128KiB read should generate 2 read responses");
```

**Step 2: Run test to verify it fails**

Run:

```bash
ninja -C cmake-cache unified-bus-test -j 1
./build/utils/ns3.44-test-runner-default --test-name=unified-bus
```

Expected: failure because receive-side tracking still conflates `MAETAH.Length` with payload bytes for `READ` requests.

**Step 3: Write minimal implementation**

In `TrackInboundTaPacket()` and the receive path:

- derive inbound payload bytes from the remaining packet size after header removal;
- set `logicalBytes` from `MAETAH.Length` for `READ` requests;
- accumulate reassembly completion using payload bytes, not logical bytes.

**Step 4: Run test to verify it passes**

Run the same commands.

Expected: exactly two request-slice completions and two responses.

**Step 5: Commit**

```bash
git add src/unified-bus/model/protocol/ub-transport.cc src/unified-bus/model/ub-datatype.h src/unified-bus/test/ub-test.cc
git commit -m "fix(urma): reassemble read requests by payload bytes"
```

### Task 4: Make transaction execution and response completion slice-aware

**Files:**
- Modify: `src/unified-bus/model/protocol/ub-function.cc`
- Modify: `src/unified-bus/model/protocol/ub-transaction.cc`
- Modify: `src/unified-bus/model/ub-datatype.h`
- Test: `src/unified-bus/test/ub-test.cc`

**Step 1: Write the failing test**

Replace the old expectation that a multi-packet read yields exactly one response. The new regression for `128KiB` should assert:

```cpp
NS_TEST_ASSERT_MSG_EQ(m_readResponseCount, 2u, "multi-slice read should generate one response per request slice");
NS_TEST_ASSERT_MSG_EQ(m_taskCompleteCount, 1u, "multi-slice read should still complete the WQE exactly once");
```

**Step 2: Run test to verify it fails**

Run:

```bash
ninja -C cmake-cache unified-bus-test -j 1
./build/utils/ns3.44-test-runner-default --test-name=unified-bus
```

Expected: failure because current code completes multi-slice reads as if there were one response or uses the wrong request identity.

**Step 3: Write minimal implementation**

Update TA semantics:

- each read request slice carries its own `requestTassn`;
- target builds one `READ_RESPONSE` per slice and echoes that `requestTassn`;
- initiator completes slices by the echoed `requestTassn`;
- Jetty callback still fires once after all slices finish.

**Step 4: Run test to verify it passes**

Run the same commands.

Expected: 2 responses for 128KiB, 1 task completion.

**Step 5: Commit**

```bash
git add src/unified-bus/model/protocol/ub-function.cc src/unified-bus/model/protocol/ub-transaction.cc src/unified-bus/model/ub-datatype.h src/unified-bus/test/ub-test.cc
git commit -m "fix(urma): complete read slices on per-slice responses"
```

### Task 5: Update docs to reflect the finalized read slicing semantics

**Files:**
- Modify: `scratch/README.md`
- Modify: `docs/plans/2026-03-19-urma-read-design.md`
- Modify: `docs/plans/2026-03-19-urma-read-write-alignment.md`

**Step 1: Write the failing doc checklist**

Add/update prose so the docs explicitly say:

- `URMA_READ` request slices carry zero payload;
- `MAETAH.Length` carries logical read size;
- each read request slice generates one `READ_RESPONSE`;
- response may span multiple TP packets.

**Step 2: Run doc verification**

Run:

```bash
rg -n "zero payload|READ_RESPONSE|request slice|MAETAH.Length" scratch/README.md docs/plans/2026-03-19-urma-read-design.md docs/plans/2026-03-19-urma-read-write-alignment.md
```

Expected: before editing, at least some of the new clarifications are missing.

**Step 3: Write minimal documentation updates**

Make the docs match the final implemented behavior without rewriting unrelated history.

**Step 4: Run doc verification**

Run the same `rg` command.

Expected: all intended clarifications appear.

**Step 5: Commit**

```bash
git add scratch/README.md docs/plans/2026-03-19-urma-read-design.md docs/plans/2026-03-19-urma-read-write-alignment.md
git commit -m "docs(urma): clarify read request slicing semantics"
```

### Task 6: Final verification

**Files:**
- Verify only

**Step 1: Build tests serially**

Run:

```bash
ninja -C cmake-cache unified-bus-test -j 1
```

Expected: build succeeds.

**Step 2: Run unified-bus tests**

Run:

```bash
./build/utils/ns3.44-test-runner-default --test-name=unified-bus
```

Expected: all unified-bus tests pass, including updated URMA read regressions.

**Step 3: Run quick example regression**

Run:

```bash
./ns3 run --no-build 'scratch/ub-quick-example --case-path=scratch/2nodes_single-tp'
```

Expected: example still succeeds.

**Step 4: Commit verification note if needed**

If code/doc changes occurred after the previous commit, create a final commit:

```bash
git add <touched files>
git commit -m "test(urma): verify read request slicing flow"
```

