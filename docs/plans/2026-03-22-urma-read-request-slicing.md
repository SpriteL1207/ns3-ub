# URMA Read Request Slicing Design Patch

> **Context:** This document patches/clarifies the previously accepted URMA read/write alignment design by
> pinning down spec-aligned **URMA_READ request slicing** semantics, especially the separation between
> transaction-layer length and on-wire payload bytes.

**Goal:** Make `URMA_READ` follow UB spec semantics for request slicing and response handling:

- A large `URMA_READ` WQE may be sliced at the transaction layer (TA slice / request slice).
- Each **request slice** produces exactly one **READ_RESPONSE** transaction unit.
- READ_RESPONSE may be transported using multiple TP packets (MTU-based packetization), but must not be
  transaction-sliced further.
- A `URMA_READ` WQE completes only when all request-slice responses are received.

---

## Spec Anchor (Paraphrased)

From UB Base Specification 2.0 (zh, section 7 transaction layer, read flow):

- Under RTP, `Read` supports **request slicing**.
- A request slice's response may span multiple TP packets.
- `Read Response` must not be transaction-sliced further.
- `Read` request format has **no payload**; the length semantics are carried in `MAETAH.Length`.

---

## Terminology

- **logicalBytes**
  Transaction semantic byte count for a transaction unit (request slice / response).
  For `URMA_READ` request slices, this is the read length.
  This value must be encoded into `MAETAH.Length`.

- **payloadBytes**
  Actual on-wire payload bytes carried in TP packets for the transaction unit.
  This controls TP packetization, congestion accounting, and TP reassembly byte counting.
  For `URMA_READ` request slices, this must be `0`.

- **carrierBytes**
  Send-side progress unit. This is not "payload"; it exists only to keep the existing TP scheduler and
  WQE-segment state machine moving when `payloadBytes == 0` but a request/response packet must still be sent.

---

## Design Constraints

### Request slicing model

- `UbJetty` continues to slice a large `URMA_READ` WQE into multiple request slices at the TA layer,
  using the existing `UB_WQE_TA_SEGMENT_BYTE` slicing boundary.
- Each request slice is modeled as one `UbWqeSegment` of kind `REQUEST`.

### One request packet per request slice, with zero payload

Even though the request slice has `logicalBytes > 0`, the on-wire request must carry **no payload**:

- `payloadBytes = 0` for `TA_OPCODE_READ` request slices.
- The request slice must still produce exactly one transmitted TP packet.

To preserve the existing send-side scheduling / state machine without introducing a separate send path,
we use a non-zero **carrierBytes**:

- For `TA_OPCODE_READ` request slices: `carrierBytes = 1`.
- For `TA_OPCODE_TRANSACTION_ACK` (TAACK): `carrierBytes = 1`.

This preserves "one packet exists to send" while staying consistent with spec payload semantics.

### Response modeling

- `TA_OPCODE_READ_RESPONSE` is a transaction-layer `RESPONSE` unit with:
  - `logicalBytes = responseBytes`
  - `payloadBytes = logicalBytes`
  - `carrierBytes = payloadBytes`
- TP may split the response payload into multiple packets up to `UB_MTU_BYTE`.
- The receiver must reassemble those packets into a single completed TA unit before passing to TA.

### Request identity and completion granularity

With request slicing, each request slice has its own Initiator TASSN (INI_TASSN), and the matching response
must echo that same INI_TASSN so the initiator can complete the correct slice.

In our current codebase, the field `requestTassn` is used as the slice identifier carried by responses.
This patch requires:

- for each `READ` request slice, `requestTassn` must be set to that slice's INI_TASSN;
- for the corresponding `READ_RESPONSE`, `requestTassn` must echo the triggering request slice INI_TASSN;
- the initiator completes the WQE only after all slice responses have been received.

### Address derivation (simplified for this iteration)

This iteration does not introduce OFSTETAH parsing/serialization.
Address derivation remains a simplified model based on TASSN (with the important constraint that it must be
stable across multiple WQEs on the same Jetty; do not use absolute TASSN alone if it can collide across WQEs).

---

## Send Path Changes

### UbWqeSegment extensions

Add explicit fields:

- `logicalBytes`
- `payloadBytes`
- `carrierBytes`

Rules:

- `WRITE` request: `logicalBytes == payloadBytes == carrierBytes == dataBytes`
- `READ` request: `logicalBytes == sliceBytes`, `payloadBytes == 0`, `carrierBytes == 1`
- `TAACK`: `logicalBytes == 0`, `payloadBytes == 0`, `carrierBytes == 1`
- `READ_RESPONSE`: `logicalBytes == payloadBytes == carrierBytes == responseBytes`

### TP packet generation

`UbTransportChannel::GenDataPacket()` must:

- create `Packet(payloadBytes)` (may be `0`)
- set `MAETAH.Length = logicalBytes` (not `payloadBytes`)

Congestion/metrics accounting that currently uses payload size must use `payloadBytes`.
Send-side progress must use `carrierBytes` so zero-payload requests still advance to completion.

---

## Receive Path Changes

The receive path must not interpret `MAETAH.Length` as on-wire payload bytes.

For each inbound packet:

- `payloadBytes` is the post-header packet size (after removing headers).
- TP reassembly byte accumulation must use `payloadBytes`.

For `TA_OPCODE_READ` request:

- `logicalBytes` is read from `MAETAH.Length`.
- `payloadBytes` must be `0`.

For `TA_OPCODE_READ_RESPONSE`:

- `logicalBytes` should match `payloadBytes` once reassembly completes.

When TP reassembly completes (LastPacket observed), deliver the completed TA unit to:

- `UbTransaction::HandleInboundTaUnit()`

---

## TA / Transaction Behavior

Target-side:

- On completed `READ` request slice, execute exactly once and build one `READ_RESPONSE`.
- Enqueue the response into `m_tpRelatedRemoteRequests` so it participates in TP scheduling.

Initiator-side:

- On receiving a completed `READ_RESPONSE`, complete exactly the corresponding request slice.
- Only after all slices are completed should the Jetty report the WQE as complete to the client callback.

---

## Test/Verification Additions

Add a regression test for a multi-slice read (e.g. `128KiB` = 2 slices) proving:

- exactly 2 target-side read request packets are received, and their payload bytes are 0 (via `TpRecvNotify`);
- exactly 2 read responses are observed as completed TA units (via last-packet receives notify per response);
- the read WQE completes exactly once, after both slice responses arrive.

