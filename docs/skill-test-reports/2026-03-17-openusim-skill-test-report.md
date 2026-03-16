# OpenUSim Skill Test Report

Date: 2026-03-17

## Overview

Three simulated conversations were run against the OpenUSim stage skill system, with increasing difficulty. Each conversation was conducted by an agent playing both the user and the AI assistant following the skills. This report consolidates findings across all three tests.

---

## Test 1: Easy — Broad Request, New User

**Scenario:** User opens with "我想做一次 openusim 仿真", no prior knowledge, accepts defaults, wants 2-layer Clos + ar_ring.

### What Worked Well

- First reply correctly stayed in planning mode, asked one goal question (AGENTS.md lines 48-59)
- `broad` classification handled correctly (AGENTS.md lines 76-77)
- "2层 Clos" correctly bound to `clos-spine-leaf` without re-asking (AGENTS.md line 80)
- Generation gate respected until explicit user approval (AGENTS.md lines 99-111)
- No skill names or routing labels echoed to user (AGENTS.md lines 28-30)
- Toolchain-native parameter names used in summary (`host_num`, `leaf_sw_num`, `comm_domain_size`) (spec-rules.md lines 83-87)

### Violations Found

| Violation | Severity | Skill Reference |
|-----------|----------|-----------------|
| Startup gate never checked — repo readiness not verified before planning or generation | High | AGENTS.md lines 113-134 |
| `experiment-spec.md` never written at any point | High | spec-rules.md lines 13-19; plan-experiment SKILL.md lines 56-63 |
| Observability slot resolved by silent assumption ("默认 trace 模式") without asking | Medium | plan-experiment SKILL.md line 29 |
| Turn 5 summary used explicit bullet labels without user requesting structured summary | Low | AGENTS.md lines 40-43 |
| Two questions asked in one turn (comm_domain_size + data_size) | Low | AGENTS.md line 87 |
| Spine count volunteered as unsolicited derived fact | Low | AGENTS.md line 87 |

---

## Test 2: Medium — Semi-Specified, fat-tree Disambiguation + Trace Question

**Scenario:** Researcher opens with "我想跑一个 fat-tree k=4 的拓扑，做 all-to-all pairwise 通信，数据量 512MB，看看吞吐量能不能接近线速". Asks about detailed trace mid-conversation. Asks about throughput results after run.

### What Worked Well

- Provided facts bound immediately before asking anything (AGENTS.md line 23; plan-experiment SKILL.md line 23)
- fat-tree disambiguation triggered correctly per topology-options.md lines 39-43 and AGENTS.md line 81
- One question per turn maintained throughout
- Trace question answered using trace-observability.md knowledge card (AGENTS.md lines 155-161); safe wording used, unsafe wording avoided (trace-observability.md lines 21-29)
- Throughput interpretation used throughput-evidence.md; per-port vs per-task distinction made correctly (throughput-evidence.md lines 7-9, 14-15)
- No line-rate claim made without stating both metric and comparison target (throughput-evidence.md lines 16-17)
- rank_mapping and phase_delay collected in planning, not deferred to runtime (workload-options.md lines 80-82)

### Violations Found

| Violation | Severity | Skill Reference |
|-----------|----------|-----------------|
| `clos-fat-tree` has no mapped example script in spec-to-toolchain.md — AI silently used `user_topo_2layer_clos.py` without flagging the gap | High | spec-to-toolchain.md lines 7-9; plan-experiment SKILL.md line 38 (Stop And Ask) |
| Startup gate never checked | High | AGENTS.md lines 113-134 |
| `experiment-spec.md` never written | High | spec-rules.md lines 13-19 |
| Turn 5 summary used markdown bold headers without user requesting structured summary | Low | AGENTS.md lines 40-43 |
| Default bandwidth/delay stated as "400Gbps, 20ns" without querying runtime catalog | Medium | AGENTS.md lines 144-151 |

---

## Test 3: Hard — Reference-Based, Invalid Constraint, Iteration, Analysis

**Scenario:** Expert user references a non-existent old case, requests invalid comm_domain_size (512 % 300 ≠ 0), asks about transport_channel.csv, iterates with scatter_k change, asks about result validity for report.

### What Worked Well

- Reference-based classification correctly identified; path not found handled gracefully without hallucinating old case content (AGENTS.md line 79)
- `512 % 300 != 0` constraint violation caught immediately with specific fix options (workload-options.md line 74)
- `scatter_k` identified as missing required parameter and asked as next blocking question (workload-options.md line 67; AGENTS.md line 87)
- `transport_channel.csv` question answered using transport-channel-modes.md; on-demand path described as valid, not broken (transport-channel-modes.md lines 8-9, 13-15, 19-21)
- Goal confirmation required before generation gate opened (AGENTS.md lines 99-111; spec-rules.md lines 67-74)
- spec-to-toolchain.md CLI format used correctly for build_traffic.py invocation (spec-to-toolchain.md lines 23-33)
- Iteration correctly identified as experiment definition change → handoff to plan stage (analyze-results SKILL.md lines 43-48)
- Final result conclusion correctly refused without flagging comm_domain_size mismatch between old and new cases (analyze-results SKILL.md line 11; throughput-evidence.md line 16)

### Violations Found

| Violation | Severity | Skill Reference |
|-----------|----------|-----------------|
| Old case path not actually checked via file system before declaring missing | Medium | AGENTS.md lines 115-134 (operating on repo directly) |
| `experiment-spec.md` not shown as written despite being mentioned | High | spec-rules.md lines 13-19 |
| Re-planning after scatter_k change did not explicitly re-confirm all planning slots | Medium | AGENTS.md lines 99-111; spec-rules.md line 97 |
| scatter_k semantic explanation included unverified inference about "phase sync overhead" | Low | CLAUDE.md meta-rules (unverified content must be flagged) |
| Startup gate never checked | High | AGENTS.md lines 113-134 |

---

## Cross-Test Summary

### Consistently Strong Behaviors

1. **Question granularity** — One blocking question per turn was maintained in all three tests. No full-form questionnaires.
2. **Generation gate** — Generation never started before explicit user approval in any test.
3. **Knowledge card routing** — trace-observability.md, throughput-evidence.md, and transport-channel-modes.md were all consulted correctly when triggered.
4. **No internal labels echoed** — Skill names, routing decisions, and control labels never appeared in user-facing replies across all tests.
5. **Constraint enforcement** — workload-options.md hard constraints (divisibility, scatter_k range) were caught correctly.
6. **Toolchain-native parameter names** — spec-rules.md parameter naming rule followed consistently.

### Consistently Failing Behaviors

| Gap | Frequency | Severity |
|-----|-----------|----------|
| Startup gate (AGENTS.md lines 113-134) never checked | All 3 tests | High |
| `experiment-spec.md` never actually written | All 3 tests | High |
| Summary/confirmation turns used labeled lists without user requesting structured output | All 3 tests | Low |

### Single-Test Failures

| Gap | Test | Severity |
|-----|------|----------|
| `clos-fat-tree` has no script mapping in spec-to-toolchain.md — AI silently bridged the gap | Test 2 | High |
| Default parameter values stated without querying runtime catalog | Test 2 | Medium |
| Old case path not actually checked via file system | Test 3 | Medium |
| Re-planning after iteration did not re-confirm all slots | Test 3 | Medium |

---

## Top Issues to Fix

### Issue 1 (Critical): Startup gate is universally skipped

**Problem:** In all three tests, the AI proceeded from the first user message directly into planning without ever checking repo readiness (AGENTS.md lines 113-134). This means generation could be approved against a repo that is not built or configured.

**Root cause:** The startup gate rule is in AGENTS.md but not enforced as a hard prerequisite in any individual SKILL.md. The welcome skill covers it, but if the user skips welcome and goes directly to planning, the gate is bypassed.

**Fix direction:** Add an explicit startup gate check to `openusim-plan-experiment/SKILL.md` and `openusim-run-experiment/SKILL.md` as a "Stop And Ask" condition: "Startup facts have not been verified for this session."

---

### Issue 2 (Critical): `experiment-spec.md` write-back never happens in practice

**Problem:** In all three tests, the AI verbally summarized the spec but never actually wrote `experiment-spec.md`. The spec-rules.md write-back timing rules (lines 13-19) and the handover requirements (plan-experiment SKILL.md lines 56-63) were consistently ignored.

**Root cause:** The skill says *when* to write back but does not say *how* — there is no concrete instruction to call a write tool or produce the file content. The AI treats the verbal summary as sufficient.

**Fix direction:** In `openusim-plan-experiment/SKILL.md`, add an explicit step: "Before handing off to openusim-run-experiment, write the confirmed slots to `experiment-spec.md` using the minimal template from spec-rules.md." Include a concrete example of the write action.

---

### Issue 3 (High): `clos-fat-tree` is a planning option with no run-stage script mapping

**Problem:** `topology-options.md` lists `clos-fat-tree` as a supported planning family, but `spec-to-toolchain.md` has no mapping for it — only `clos-spine-leaf` and `nd-full-mesh` have example scripts. When a user selects `clos-fat-tree`, the AI silently falls back to `user_topo_2layer_clos.py` without flagging the gap.

**Root cause:** The planning and run-stage references are inconsistent. `topology-options.md` was written from a planning perspective; `spec-to-toolchain.md` was written from a run perspective, and the two were not reconciled.

**Fix direction:** Either (a) add a `clos-fat-tree` script mapping to `spec-to-toolchain.md` with a concrete example script, or (b) add a note to `topology-options.md` that `clos-fat-tree` requires a custom script and the AI must ask the user to confirm before proceeding. Also add a "Stop And Ask" condition to `openusim-run-experiment/SKILL.md`: "The topology family has no mapped example script in spec-to-toolchain.md."

---

### Issue 4 (Medium): Default parameter values stated without runtime catalog query

**Problem:** In Test 2, the AI stated "400Gbps, 20ns" as default values without querying the runtime catalog via `--PrintTypeIds` / `--ClassName` / `--PrintUbGlobals`. AGENTS.md lines 144-151 explicitly prohibits relying on a hand-written static parameter table.

**Root cause:** The runtime catalog query is described in AGENTS.md but is not referenced in `openusim-plan-experiment/SKILL.md` or `openusim-run-experiment/SKILL.md`. The AI has no reminder to query the catalog before stating defaults.

**Fix direction:** Add a note to `openusim-run-experiment/SKILL.md` Process section: "Before stating default parameter values, verify against the runtime catalog (load_or_build_parameter_catalog()) rather than reciting static values."

---

### Issue 5 (Medium): Confirmation summaries consistently violate implicit-structure rule

**Problem:** In all three tests, the AI used labeled bullet lists or bold headers in confirmation summaries without the user requesting a structured summary. AGENTS.md lines 40-43 requires keeping structure implicit and only switching to explicit headings when the user asks.

**Root cause:** The rule exists in AGENTS.md but is easy to violate because a labeled summary feels helpful. The skill does not provide a concrete example of what an implicit confirmation looks like.

**Fix direction:** Add a concrete example to AGENTS.md of an implicit confirmation (e.g., "好，拓扑、负载和参数都定下来了，可以生成了吗？") vs a labeled one. The current guidance is abstract.

---

## Optimization Targets

Based on the above, the following changes are recommended (in priority order):

1. **`openusim-plan-experiment/SKILL.md`** — Add startup gate as explicit Stop And Ask condition; add concrete write-back instruction for `experiment-spec.md` before handoff
2. **`openusim-run-experiment/SKILL.md`** — Add startup gate as explicit Stop And Ask condition; add note about runtime catalog for default values; add Stop And Ask for unmapped topology families
3. **`spec-to-toolchain.md`** — Add `clos-fat-tree` mapping or explicit note about missing script
4. **`topology-options.md`** — Add note to `clos-fat-tree` entry that no example script exists and agent must surface this gap
5. **`AGENTS.md`** — Add concrete example of implicit confirmation wording to the User-Facing Reply Surface section
