# URMA Read/Write Transaction Alignment Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Add spec-aligned `URMA_READ` support to `unified-bus` and make `URMA_WRITE` complete only after a transaction-layer response arrives.

**Architecture:** Keep asynchronous URMA on the existing `Jetty -> TA -> TP` path. `UbTransportChannel` remains responsible for packetization, per-packet ACKs, and receive-side TA-unit reassembly, while `UbTransaction` owns target-side execution, abstract memory, response generation, and initiator-side request completion. Remote responses must reuse `m_tpRelatedRemoteRequests`; do not add a second response send path or any new trace sources.

**Tech Stack:** C++20, ns-3 object model, unified-bus protocol stack, `ninja`, `ns3.44-test-runner`, quick-example system tests

---

Use `@superpowers:test-driven-development` inside each task and `@superpowers:verification-before-completion` before declaring the work finished.

## Guardrails

- Keep builds serial. Do not run parallel `ninja`, `./ns3 build`, or `./test.py` for this change.
- Keep `traffic.csv` unchanged. Do not add user-facing `remoteAddress`, `tokenId`, `localAddress`, or `readOffset`.
- Model `tokenId` as fixed `0` if a field is needed internally. Transaction execution latency stays `0ns`.
- Scope is ROI success path only. Reject non-ROI URMA read/write explicitly instead of silently pretending support.
- Do not add new trace sources. Tests must rely on existing callbacks, state, and narrow read-only helpers only if they are needed to observe correctness.

### Task 1: Front-End URMA Read Scaffolding

**Files:**
- Modify: `src/unified-bus/model/ub-app.cc`
- Modify: `src/unified-bus/model/ub-app.h`
- Modify: `src/unified-bus/model/ub-traffic-gen.h`
- Modify: `src/unified-bus/model/protocol/ub-function.cc`
- Modify: `src/unified-bus/model/protocol/ub-function.h`
- Modify: `src/unified-bus/model/ub-datatype.h`
- Modify: `src/unified-bus/model/protocol/ub-transaction.cc`
- Modify: `src/unified-bus/model/protocol/ub-transaction.h`
- Test: `src/unified-bus/test/ub-test.cc`

**Step 1: Write the failing system test**

Add a quick-example test that injects a temporary `traffic.csv` containing one `URMA_READ` and expects the run to succeed.

```cpp
const std::string trafficCsv =
    "taskId,sourceNode,destNode,dataSize(Byte),opType,priority,delay,phaseId,dependOnPhases\n"
    "0,0,3,4096,URMA_READ,7,10ns,0,\n";

NS_TEST_ASSERT_MSG_EQ(status, 0, "URMA_READ should be accepted by ub-quick-example");
NS_TEST_ASSERT_MSG_NE(output.find("TEST : 00000 : PASSED"),
                      std::string::npos,
                      "URMA_READ smoke case should report PASSED");
```

**Step 2: Build the test target**

Run: `ninja -C cmake-cache unified-bus-test`

Expected: build succeeds.

**Step 3: Run the suite and confirm the new test fails**

Run: `./build/utils/ns3.44-test-runner-default --test-name=unified-bus`

Expected: FAIL because `UbApp::SendTraffic()` currently only accepts `URMA_WRITE` and falls into `NS_ASSERT_MSG(0, "TaOpcode Not Exist")`.

**Step 4: Implement the minimal front-end and metadata scaffolding**

Make these production changes:

- Add `URMA_READ` to the `TaOpcodeMap` instances in `UbApp` and `UbTrafficGen`.
- Change `UbFunction::CreateWqe()` to accept a `TaOpcode` argument and pass `TA_OPCODE_WRITE` or `TA_OPCODE_READ` from `UbApp::SendTraffic()`.
- Extend `UbWqe` and `UbWqeSegment` with the internal metadata needed by later tasks:

```cpp
enum class UbTransactionSegmentKind : uint8_t
{
    REQUEST,
    RESPONSE,
};

// UbWqe / UbWqeSegment additions
UbTransactionSegmentKind m_segmentKind{UbTransactionSegmentKind::REQUEST};
uint32_t m_originJettyNum{0};
uint32_t m_requestTassn{0};
TaOpcode m_requestOpcode{TaOpcode::TA_OPCODE_WRITE};
uint32_t m_responseBytes{0};
uint64_t m_remoteAddress{0};
bool m_needsTransactionResponse{true};
```

- Initialize those fields in `UbFunction::CreateWqe()` and `UbJetty::GenWqeSegment()`.
- Add an explicit ROI-only guard in `UbTransaction` before URMA read/write scheduling continues.

**Step 5: Rebuild**

Run: `ninja -C cmake-cache unified-bus-test`

Expected: build succeeds with the new `URMA_READ` path and metadata fields.

**Step 6: Re-run the suite**

Run: `./build/utils/ns3.44-test-runner-default --test-name=unified-bus`

Expected: the new `URMA_READ` acceptance test passes, while no transaction-response semantics are enforced yet.

**Step 7: Commit**

```bash
git add src/unified-bus/model/ub-app.cc \
        src/unified-bus/model/ub-app.h \
        src/unified-bus/model/ub-traffic-gen.h \
        src/unified-bus/model/protocol/ub-function.cc \
        src/unified-bus/model/protocol/ub-function.h \
        src/unified-bus/model/ub-datatype.h \
        src/unified-bus/model/protocol/ub-transaction.cc \
        src/unified-bus/model/protocol/ub-transaction.h \
        src/unified-bus/test/ub-test.cc
git commit -m "feat(urma): add read request scaffolding"
```

### Task 2: Make URMA Write Complete on Transaction Response

**Files:**
- Modify: `src/unified-bus/model/protocol/ub-transport.h`
- Modify: `src/unified-bus/model/protocol/ub-transport.cc`
- Modify: `src/unified-bus/model/protocol/ub-transaction.h`
- Modify: `src/unified-bus/model/protocol/ub-transaction.cc`
- Modify: `src/unified-bus/model/protocol/ub-function.h`
- Modify: `src/unified-bus/model/protocol/ub-function.cc`
- Modify: `src/unified-bus/model/ub-datatype.h`
- Test: `src/unified-bus/test/ub-test.cc`
- Reference only: `src/unified-bus/examples/ub-mtp-remote-tp-regression.cc`

**Step 1: Write the failing regression test**

Adapt the fixed-TP topology pattern from `ub-mtp-remote-tp-regression.cc` into `ub-test.cc`. Send one `64KiB` `URMA_WRITE` and assert that request TP ACK arrival does not complete the WQE.

```cpp
NS_TEST_ASSERT_MSG_EQ(senderObservedRequestTpAck, true,
                      "request TP ACK should still arrive");
NS_TEST_ASSERT_MSG_EQ(senderCompletedBeforeTaResponse, false,
                      "URMA_WRITE must not complete on request TP ACK");
NS_TEST_ASSERT_MSG_EQ(senderObservedTaAck, true,
                      "target must generate one TAACK response");
NS_TEST_ASSERT_MSG_EQ(senderCompletedAfterTaAck, true,
                      "URMA_WRITE should complete on TAACK");
```

If the test needs observability beyond existing callbacks, add a narrow read-only counter in `UbTransaction`; do not add any trace source.

**Step 2: Build the test target**

Run: `ninja -C cmake-cache unified-bus-test`

Expected: build succeeds.

**Step 3: Run the suite and confirm the new test fails**

Run: `./build/utils/ns3.44-test-runner-default --test-name=unified-bus`

Expected: FAIL because `UbTransportChannel::RecvTpAck()` still calls `UbTransaction::ProcessWqeSegmentComplete()` for the original request segment as soon as request packets are ACKed.

**Step 4: Implement TA-unit reassembly, target execution, and TAACK completion**

Make these production changes:

- In `UbTransportChannel`, add receive-side TA-unit reassembly keyed by `(srcTpn, tpMsn)`.
- Keep per-packet TP ACK generation exactly where it belongs: in `RecvDataPacket()`.
- When the last packet of a TA unit arrives, hand the completed TA request upward to `UbTransaction` instead of pretending TP ACK means TA completion.
- In `UbTransaction`, add request/response handling methods along these lines:

```cpp
void HandleInboundTaUnit(uint32_t localTpn, Ptr<UbWqeSegment> segment);
Ptr<UbWqeSegment> ExecuteRemoteWriteAndBuildAck(uint32_t localTpn, Ptr<UbWqeSegment> request);
void CompleteLocalRequestFromResponse(Ptr<UbWqeSegment> response);
```

- Back the target-side write executor with an abstract memory store keyed by `(targetNodeId, derivedAddress)`.
- Build one `TA_OPCODE_TRANSACTION_ACK` response segment with:

```cpp
response->SetSegmentKind(UbTransactionSegmentKind::RESPONSE);
response->SetType(TaOpcode::TA_OPCODE_TRANSACTION_ACK);
response->SetOriginJettyNum(request->GetOriginJettyNum());
response->SetRequestTassn(request->GetRequestTassn());
response->SetRequestOpcode(TaOpcode::TA_OPCODE_WRITE);
response->SetResponseBytes(0);
```

- Enqueue that response via `m_tpRelatedRemoteRequests[replyTpn][originJettyNum]`.
- Split completion semantics cleanly:
  - request TP ACK retires TP send state only;
  - initiator-side `TAACK` advances Jetty/WQE completion;
  - TP ACK for the response segment retires target-side TP send state only.

**Step 5: Rebuild**

Run: `ninja -C cmake-cache unified-bus-test`

Expected: build succeeds with the new TA response path.

**Step 6: Re-run the suite**

Run: `./build/utils/ns3.44-test-runner-default --test-name=unified-bus`

Expected: the write regression passes and `URMA_WRITE` completion is now response-driven.

**Step 7: Commit**

```bash
git add src/unified-bus/model/protocol/ub-transport.h \
        src/unified-bus/model/protocol/ub-transport.cc \
        src/unified-bus/model/protocol/ub-transaction.h \
        src/unified-bus/model/protocol/ub-transaction.cc \
        src/unified-bus/model/protocol/ub-function.h \
        src/unified-bus/model/protocol/ub-function.cc \
        src/unified-bus/model/ub-datatype.h \
        src/unified-bus/test/ub-test.cc
git commit -m "fix(urma): complete writes on ta responses"
```

### Task 3: Add URMA Read Request/Response Execution

**Files:**
- Modify: `src/unified-bus/model/protocol/ub-transport.h`
- Modify: `src/unified-bus/model/protocol/ub-transport.cc`
- Modify: `src/unified-bus/model/protocol/ub-transaction.h`
- Modify: `src/unified-bus/model/protocol/ub-transaction.cc`
- Modify: `src/unified-bus/model/protocol/ub-function.h`
- Modify: `src/unified-bus/model/protocol/ub-function.cc`
- Modify: `src/unified-bus/model/ub-datatype.h`
- Test: `src/unified-bus/test/ub-test.cc`

**Step 1: Write the failing regression tests**

Add three tests:

- a direct fixed-TP regression proving one `URMA_READ` completes only after one `READ_RESPONSE`;
- a `128KiB` direct regression proving a multi-packet read request triggers exactly one target-side transaction execution;
- a quick-example system test with dependent phases `URMA_WRITE -> URMA_READ`.

Use assertions like:

```cpp
NS_TEST_ASSERT_MSG_EQ(senderObservedReadResponse, true,
                      "URMA_READ should generate a READ_RESPONSE");
NS_TEST_ASSERT_MSG_EQ(senderCompletedOnReadResponse, true,
                      "URMA_READ must complete on READ_RESPONSE");
NS_TEST_ASSERT_MSG_EQ(targetReadExecutionCount, 1,
                      "multi-packet read request must execute once at the TA layer");
```

**Step 2: Build the test target**

Run: `ninja -C cmake-cache unified-bus-test`

Expected: build succeeds.

**Step 3: Run the suite and confirm the new tests fail**

Run: `./build/utils/ns3.44-test-runner-default --test-name=unified-bus`

Expected: FAIL because there is still no target-side `READ_RESPONSE` generation or initiator-side read completion path.

**Step 4: Implement remote read execution and response matching**

Make these production changes:

- Extend the abstract memory store in `UbTransaction` to serve reads as well as writes.
- On a reassembled `TA_OPCODE_READ` request, execute exactly once at the target and build one `TA_OPCODE_READ_RESPONSE` segment.
- Populate read-response metadata so the initiator can match it back to the original request:

```cpp
response->SetSegmentKind(UbTransactionSegmentKind::RESPONSE);
response->SetType(TaOpcode::TA_OPCODE_READ_RESPONSE);
response->SetOriginJettyNum(request->GetOriginJettyNum());
response->SetRequestTassn(request->GetRequestTassn());
response->SetRequestOpcode(TaOpcode::TA_OPCODE_READ);
response->SetResponseBytes(request->GetSize());
```

- Fill missing bytes from abstract memory with zero.
- Reuse the same `m_tpRelatedRemoteRequests` queue and `ScheduleWqeSegment()` path as write responses.
- At the initiator, match inbound `READ_RESPONSE` by `originJettyNum + requestTassn`, then complete the original read WQE.
- Keep response-packet TP ACK semantics local to the responder; they must not double-complete the initiator request.

**Step 5: Rebuild**

Run: `ninja -C cmake-cache unified-bus-test`

Expected: build succeeds with the new read path.

**Step 6: Re-run the suite**

Run: `./build/utils/ns3.44-test-runner-default --test-name=unified-bus`

Expected: all read-specific regressions pass, including the multi-packet read execution count check.

**Step 7: Commit**

```bash
git add src/unified-bus/model/protocol/ub-transport.h \
        src/unified-bus/model/protocol/ub-transport.cc \
        src/unified-bus/model/protocol/ub-transaction.h \
        src/unified-bus/model/protocol/ub-transaction.cc \
        src/unified-bus/model/protocol/ub-function.h \
        src/unified-bus/model/protocol/ub-function.cc \
        src/unified-bus/model/ub-datatype.h \
        src/unified-bus/test/ub-test.cc
git commit -m "feat(urma): add read response path"
```

### Task 4: End-to-End Quick-Example Coverage and Documentation

**Files:**
- Modify: `src/unified-bus/test/ub-test.cc`
- Modify: `scratch/README.md`

**Step 1: Add the final quick-example system tests**

Add temporary-case tests for:

- single `URMA_WRITE`;
- single `URMA_READ`;
- dependent `URMA_WRITE` then `URMA_READ`;
- a small mixed concurrent read/write workload.

Keep them in `ub-test.cc` by reusing `CopyCaseDirWithTrafficFile(...)`; do not create permanent new scratch cases just for verification.

```cpp
traffic << "0,0,3,4096,URMA_WRITE,7,10ns,10,\n";
traffic << "1,0,3,4096,URMA_READ,7,10ns,20,10\n";
```

**Step 2: Build the test target**

Run: `ninja -C cmake-cache unified-bus-test`

Expected: build succeeds.

**Step 3: Run the suite**

Run: `./build/utils/ns3.44-test-runner-default --test-name=unified-bus`

Expected: PASS. If a new end-to-end regression appears here, fix only the minimal production code in files already touched by Tasks 1-3 before continuing.

**Step 4: Update the user-facing documentation**

Update `scratch/README.md` so the `traffic.csv` section explicitly lists `URMA_READ` as a supported `opType`, and document these constraints:

- no extra `traffic.csv` fields are required in this iteration;
- URMA read/write completion is transaction-response-driven, not request-TP-ACK-driven;
- only ROI success-path semantics are implemented.

**Step 5: Rebuild**

Run: `ninja -C cmake-cache unified-bus-test`

Expected: build succeeds.

**Step 6: Run final verification**

Run: `./build/utils/ns3.44-test-runner-default --test-name=unified-bus`

Expected: PASS for the full `unified-bus` suite.

Run: `./ns3 run --no-build 'scratch/ub-quick-example --case-path=scratch/ub-local-hybrid-minimal'`

Expected: PASS as a baseline smoke check for the existing quick-example flow.

**Step 7: Commit**

```bash
git add src/unified-bus/test/ub-test.cc scratch/README.md
git commit -m "test(urma): cover read write quick-example flows"
```

## Knowledge Sources

- `docs/plans/2026-03-19-urma-read-design.md`
- `UB-Base-Specification-2.0-zh.pdf`
- `src/unified-bus/model/protocol/ub-transaction.{h,cc}`
- `src/unified-bus/model/protocol/ub-transport.{h,cc}`
- `src/unified-bus/model/protocol/ub-function.{h,cc}`
- `src/unified-bus/model/ub-app.cc`
- `src/unified-bus/test/ub-test.cc`
