# URMA Read Design

## Goal

Align the `unified-bus` URMA path with the UB specification for asynchronous Jetty-based transactions by:

- correcting the current `URMA_WRITE` completion model so write completion is driven by transaction response rather than TP packet ACK alone;
- introducing spec-aligned `URMA_READ` as `Read Request + Read Response` over the existing `Jetty -> TA -> TP` path;
- reusing the existing transaction-layer remote-response scheduling hook instead of creating a second send path.

## Finalized Slice Semantics

The implemented `URMA_READ` behavior is:

- TA still slices a large read WQE into request slices.
- Each read request slice sends exactly one TP request packet with zero wire payload.
- `MAETAH.Length` on that request packet carries the logical read size of the slice, not the wire payload size.
- When TP receives a complete read request slice, it hands that slice to `UbTransaction`, which enqueues exactly one `READ_RESPONSE` through `m_tpRelatedRemoteRequests`.
- The `READ_RESPONSE` may span multiple TP packets, but it is not split into multiple TA responses for the same request slice.
- The initiator completes the WQE only after all request-slice responses arrive.

## Current-State Findings

### Current `URMA_WRITE` path

`URMA_WRITE` currently enters `UbApp`, creates a Jetty/WQE, binds TP channels, and sends data through the URMA/IP path:

- `UbApp::SendTraffic()` handles `URMA_WRITE`
- `UbFunction` creates and enqueues `UbWqe`
- `UbTransaction` schedules `UbWqeSegment`
- `UbTransportChannel` packetizes and sends TP packets
- `UbSwitch` dispatches URMA/IP packets to TP receive handlers

This means `URMA_WRITE` is already using the Jetty/TA/TP stack, not the compact LDST path.

### Current modeling gaps

The current URMA path is closer to "reliable write traffic over TP" than a complete URMA transaction model.

Main gaps:

1. Completion semantics are wrong.
   The sender completes a WQE when TP-level ACK state advances. This models packet delivery, not transaction completion.

2. Target-side transaction execution is missing.
   The receive side accepts TP packets and generates TP ACKs, but does not execute a write/read transaction and generate a transaction response.

3. Read is absent from the URMA path.
   `READ_RESPONSE` exists only in the compact LDST path, not in the Jetty/TA/TP path.

4. Request metadata is incomplete.
   The current URMA request path does not meaningfully model target memory address, response matching, or request/response role separation.

5. Transaction service modes are only partially real.
   The code exposes ROI/ROT/ROL/UNO concepts, but only a thin ROI-like scheduling behavior is materially present.

## Approaches Considered

### Approach A: Complete request/response in the transaction layer

Use the existing `Jetty -> TA -> TP` path for both write and read requests. After TP reassembly at the target, hand the completed request to `UbTransaction`, execute the abstract transaction there, and enqueue a remote transaction response back through TP using `m_tpRelatedRemoteRequests`.

Pros:

- matches the UB transaction/transport layering;
- keeps TP responsible for transport, not transaction execution;
- naturally supports both `TAACK` and `READ_RESPONSE`;
- reuses the existing remote-response scheduling hook.

Cons:

- requires introducing target-side transaction execution and response bookkeeping;
- requires separating TP ACK from transaction completion.

### Approach B: Special-case read/write in TP receive logic

Let TP directly interpret completed request payloads and generate `TAACK`/`READ_RESPONSE` itself.

Pros:

- fewer touched components;
- faster to wire in initially.

Cons:

- collapses transaction semantics into the transport layer;
- makes future write-notify, atomic, richer error responses, and service-mode behavior harder;
- works against the repository's stated layering direction.

### Approach C: Wrap the compact LDST path with a URMA-like interface

Reuse `UbLdstApi` / `MEM_LOAD` / `MEM_STORE` and add a thin URMA facade.

Pros:

- lowest initial implementation cost.

Cons:

- does not model async Jetty transactions;
- mixes two protocol paths with different packet formats and semantics;
- does not satisfy the goal of a spec-aligned URMA read path.

### Recommendation

Choose Approach A.

It is the only option that keeps the model coherent with the specification and with the existing Jetty/TA/TP architecture, while also making the existing remote-response hook meaningful.

## Chosen Design

## Architecture Boundary

Responsibilities after the change:

- `UbApp` / `UbFunction` / `UbJetty`
  Submit local URMA transactions and maintain local WQE lifecycle.

- `UbTransaction`
  Own transaction-layer semantics: request identity, target-side transaction execution, response generation, and mapping responses back to the original request.

- `UbTransportChannel`
  Own TP packetization, TP reliability, TP receive-side reassembly, and delivery of fully reassembled transaction units upward.

- `UbSwitch`
  Continue routing URMA/IP traffic to TP and compact UB memory traffic to LDST.

The design intentionally avoids introducing another send path for responses. Target-generated responses are fed back into the existing TP scheduler through the transaction layer's remote-request/response queue.

## Data Model

### Input model

`traffic.csv` and `TrafficRecord` remain unchanged in this iteration.

No new user-facing fields are added for:

- `remoteAddress`
- `tokenId`
- `localAddress`
- `readOffset`

This preserves compatibility with existing scenarios and tests.

### Internal transaction metadata

To support request/response modeling, internal URMA request/segment objects need additional metadata beyond the current source/destination/size/opcode fields.

Required internal metadata:

- `segmentKind`
  Distinguishes local request segments from target-generated response segments.

- `originJettyNum`
  Identifies which initiator Jetty owns the original request so the response can complete the right local WQE.

- `requestTassn`
  Stable request identifier used to match `TAACK` / `READ_RESPONSE` back to the original request.

- `requestOpcode`
  Distinguishes whether the original request was `WRITE` or `READ`.

- `responseBytes`
  Tracks the response payload size for read responses.

- `needsTransactionResponse`
  Present for clarity, but fixed to `true` for `URMA_WRITE` and `URMA_READ` in this iteration.

Optional metadata such as access token semantics is intentionally deferred.

### Address and token handling

The UB specification requires memory access headers to carry address/token/length information.

This iteration models them as follows:

- `tokenId` is fixed to `0`;
- the request target address is internally derived rather than supplied by the scenario input;
- write multi-packet requests advance the modeled address by TP payload size;
- read requests derive a stable address the same way, so request/response matching and abstract memory access remain deterministic.

This keeps the transaction format coherent without expanding scenario inputs yet.

## Request and Response Flow

### Write

1. Initiator submits `URMA_WRITE` through Jetty.
2. TA splits the WQE into `UbWqeSegment`s.
3. TP packetizes and transports the request.
4. Target TP reassembles the request into a complete transaction unit.
5. Target `UbTransaction` executes the abstract write against target-side abstract memory.
6. Target `UbTransaction` generates a response segment with `TAACK`.
7. The response segment is inserted into `m_tpRelatedRemoteRequests`.
8. Target TP schedules and sends the response.
9. Initiator TP reassembles the response and delivers it to `UbTransaction`.
10. Initiator `UbTransaction` completes the original write WQE.

### Read

1. Initiator submits `URMA_READ` through Jetty.
2. TA splits the read request if needed.
3. Each read request slice is sent as one TP request packet with zero wire payload, while `MAETAH.Length` carries the logical slice size.
4. Target TP reassembles the request into a complete transaction unit and notifies `UbTransaction`.
5. Target `UbTransaction` executes an abstract read from target-side abstract memory.
6. Target `UbTransaction` generates one `READ_RESPONSE` segment for that request slice.
7. The response segment is inserted into `m_tpRelatedRemoteRequests`.
8. Target TP schedules and sends the response. That response may span multiple TP packets.
9. Initiator TP reassembles the response and delivers it to `UbTransaction`.
10. Initiator `UbTransaction` completes the original read WQE only after all request-slice responses are complete.

### Remote-response scheduling

The existing transaction-layer structure:

- `m_tpRelatedRemoteRequests`

is used as the target-side response injection point.

In this design it is treated as a queue of target-generated response work that competes fairly with local Jetty work during TP scheduling.

No additional bypass sender is introduced.

## Completion Semantics

This is the most important behavior change.

### Before

The sender effectively completes a local URMA WQE when transport ACK state indicates the request packets were delivered.

### After

There are three distinct events:

1. Request TP ACK received
   Means the request packets reached the peer TP receiver.
   Does not complete the transaction.

2. Response generated and sent
   Means the target transaction layer has executed the request and created a transaction response.

3. Response received at initiator
   This completes the original URMA transaction.

Completion rules:

- `URMA_WRITE` completes only when `TAACK` is received.
- `URMA_READ` completes only when `READ_RESPONSE` is received.
- For a multi-slice `URMA_READ`, each request slice completes on its matching `READ_RESPONSE`, and the WQE callback still fires once after the last slice completes.
- TP ACK for a target-generated response only releases target-side transport resources; it must not complete the original initiator request.

## Transport-Layer Changes

TP must gain a receive-side reassembly context for transaction units.

Suggested reassembly key:

- `(srcTpn, tpMsn)`

Reassembly responsibilities:

- collect request packets until the TP last-packet marker indicates the request is complete;
- collect response packets until the TP last-packet marker indicates the response is complete;
- only after reassembly is complete, pass the transaction unit to `UbTransaction`.

This keeps TP responsible for transport completeness and leaves transaction semantics above TP.

## Service Modes and Ordering Scope

This iteration supports only:

- ROI
- normal success path

Not supported in this iteration:

- ROT semantics
- ROL semantics
- UNO semantics for URMA read/write
- TP-ACK-carried transaction completion
- SCID/RCID resource behaviors
- cross-transaction RO/SO ordering guarantees beyond current baseline fields

Behavior for unsupported non-ROI URMA read/write should be explicit failure, not silent fallback that pretends semantics are implemented.

## Abstract Memory Model

The target-side transaction executor uses a lightweight abstract memory store.

Properties:

- write updates an abstract byte store indexed by target node and derived address;
- read fetches bytes from that byte store;
- missing bytes return zero-fill in this iteration;
- no local DMA/writeback model is introduced at the initiator.

This keeps the read/write request/response path testable without prematurely building a full memory subsystem.

## Timing Model

Target-side transaction execution time is modeled as `0ns` in this iteration.

End-to-end latency therefore includes:

- request transport time;
- response transport time.

It does not include:

- target execution delay;
- target memory latency;
- initiator local writeback latency.

This is an explicit simplifying assumption, not an accidental omission.

## Error Model

The target-side transaction executor returns only:

- success
- generic failure

Initial response behavior:

- write failure returns abnormal `TAACK`;
- read failure returns abnormal `READ_RESPONSE` without valid payload.

Detailed status taxonomy such as RNR / page-fault / permission faults is deferred.

## Validation Strategy

No new trace sources are added in this iteration.

Validation relies on unit tests, system tests, existing completion callbacks, and state assertions.

### Unit-test focus

Add or extend tests to verify:

1. `URMA_WRITE` does not complete on TP ACK alone.
2. `URMA_WRITE` completes on `TAACK`.
3. `URMA_READ` completes on `READ_RESPONSE`.
4. Multi-packet read requests trigger exactly one target-side transaction execution.
5. Response TP ACK frees target-side response send state but does not incorrectly complete initiator-side request state.

### System-test focus

Use minimal quick-example style scenarios for:

- single `URMA_WRITE`
- single `URMA_READ`
- `WRITE` then `READ`
- a small set of concurrent reads and writes

### Verification commands

Keep verification module-scoped and serial:

```bash
ninja -C cmake-cache unified-bus-test
./build/utils/ns3.44-test-runner-default --test-name=unified-bus
./ns3 run 'scratch/ub-quick-example --case-path=<case>'
```

## Knowledge Sources

- `UB-Base-Specification-2.0-zh.pdf`
  Used to ground transport mode, TP/TA interaction, write transaction, and read transaction semantics.

- `src/unified-bus/model/protocol/ub-transaction.{h,cc}`
  Used to identify the existing transaction scheduler and the dormant remote-response queue hook.

- `src/unified-bus/model/protocol/ub-transport.{h,cc}`
  Used to identify current TP packetization, ACK handling, and the gap between TP ACK and transaction completion.

- `src/unified-bus/model/protocol/ub-function.{h,cc}`
  Used to inspect current Jetty/WQE/WQE-segment behavior.

- `src/unified-bus/model/ub-app.cc`
  Used to confirm the current `URMA_WRITE` entry path.

- `src/unified-bus/model/ub-switch.cc`
  Used to confirm URMA packets currently terminate at TP and compact UB memory packets terminate at LDST.
