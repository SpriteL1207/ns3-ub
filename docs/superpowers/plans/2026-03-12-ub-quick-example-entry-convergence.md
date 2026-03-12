# UB Quick Example Entry Convergence Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make `ub-quick-example` the clear unified user entry, keep `UbUtils` as config-driven builder/runtime wiring, and clean unrelated dirty boundaries from this branch.

**Architecture:** Keep entry orchestration in `ub-quick-example.cc`. Keep builder/wiring primitives in `ub-utils.{h,cc}`. Rename misleading traffic-loading APIs to reflect side effects instead of pretending to be pure readers. Revert unrelated dirty headers from this checkpoint.

**Tech Stack:** ns-3 C++17, unified-bus module, system tests

---

## Chunk 1: Converge entry/builder naming

**Files:**
- Modify: `src/unified-bus/model/ub-utils.h`
- Modify: `src/unified-bus/model/ub-utils.cc`
- Modify: `src/unified-bus/examples/ub-quick-example.cc`
- Modify: `scratch/ub-quick-example.cc`

- [ ] Rename traffic-loading API to reflect configuration side effects.
- [ ] Update entry call sites to use the new name.
- [ ] Keep `ub-quick-example` orchestration explicit and unchanged in behavior.

## Chunk 2: Clean unrelated dirty boundaries

**Files:**
- Revert if unrelated: `src/unified-bus/model/protocol/ub-transport.h`
- Revert if unrelated: `src/unified-bus/model/ub-queue-manager.h`
- Revert if unrelated: `src/unified-bus/model/ub-tag.h`

- [ ] Verify these header edits are unrelated to the entry convergence.
- [ ] Revert them from the working tree if they are out of scope.

## Chunk 3: Verify and checkpoint

**Files:**
- Verify touched files only

- [ ] Build `ub-quick-example` and `unified-bus-test`.
- [ ] Run `unified-bus-examples` suite.
- [ ] Commit only this convergence checkpoint.
