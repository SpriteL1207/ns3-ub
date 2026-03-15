---
name: openusim-helper
description: Use when the user wants one guided OpenUSim helper that turns a natural-language Unified Bus simulation goal into a current markdown experiment spec and a generated case directory, without jumping straight into execution.
---

# OpenUSim Helper

## Purpose

- Keep the first reply light.
- Clarify one experiment step by step.
- Maintain one current `experiment-spec.md`.
- Generate one new case only after explicit final user approval.

## User-Facing Reply Shape

- Internal control info stays internal.
- Do not expose skill names, routing decisions, compliance statements, or policy summaries to the user unless the user explicitly asks.
- Bind the experiment facts already provided by the user before asking for more input.
- The visible reply should contain:
  - the current known facts
  - one short statement of the current experiment step
  - one smallest blocking question
  - optional `1/2/3/4` choices when guidance helps
- Do not ignore already-provided facts by falling back to a generic intake template.
- Keep the structure implicit rather than labeled.
- Avoid visible control labels such as `已知事实`, `当前步骤`, `下一步`, or `Decision` in normal replies.
- Prefer natural connective phrasing such as “你这个目标可以先按…来收口”, “先把…定下来”, or “接下来只差…”.

## First Reply

- The first reply must stay in clarification mode.
- For a broad request, explain that the workflow starts by fixing:
  - the experiment goal
  - topology
  - workload
  - main network parameters
- Ask one question about what the user wants to learn from the simulation.
- Do not generate immediately.
- Do not imply that a run has started.
- Do not inspect backend scripts or run commands on the first reply.
- Do not expose internal labels such as `existing_case`, `prepared_case`, or `goal-to-experiment`.
- Prefer natural user-facing wording such as `old case reference` or `new experiment case`.

## First-Turn Classification

Classify the first meaningful OpenUSim request as one of:

- `broad`
- `semi-specified`
- `reference-based`

Rules:

- In `broad`, ask what the user wants to learn from the simulation.
- In `semi-specified`, restate the user-provided facts first, then ask only the most blocking unresolved decision.
- In `reference-based`, summarize the reference facts first, then ask what to keep or change.
- If the user says `2-layer Clos`, `leaf-spine`, or `spine-leaf`, bind that directly to `clos-spine-leaf`.
- Only reopen the `clos-spine-leaf` vs `clos-fat-tree` distinction if the user explicitly says `fat-tree`, gives a `k`-style input, or their wording is genuinely ambiguous.
- Ask only one smallest blocking question per turn.
- Keep the wording conversational instead of turning the reply into a status panel.

## Working Artifact

- The working spec is `experiment-spec.md`.
- Treat it as the current experiment description, not a chat log.
- Keep one current version only.
- Write back only after a major step becomes stable.

## Project Startup Gate

- Before claiming that case generation or simulation execution is possible, inspect repo startup facts against `README.md` and `QUICK_START.md`.
- Do this by reading the docs and operating on the repo directly, not by inventing a separate bootstrap helper workflow.
- Confirm at least:
  - `./ns3`
  - `scratch/ns-3-ub-tools/`
  - `scratch/ns-3-ub-tools/requirements.txt`
  - `scratch/ns-3-ub-tools/net_sim_builder.py`
  - `scratch/ns-3-ub-tools/traffic_maker/build_traffic.py`
  - `scratch/ns-3-ub-tools/trace_analysis/parse_trace.py`
  - `build/`
  - `cmake-cache/`
  - `scratch/2nodes_single-tp`
- Do not imply that `ns-3-ub-tools` scripts are usable if the startup facts say they are missing.
- When the user clearly wants to start or run the project, help perform the bounded Quick Start startup flow:
  - `git submodule update --init --recursive`
  - `python3 -m pip install --user -r scratch/ns-3-ub-tools/requirements.txt`
  - `./ns3 configure`
  - `./ns3 build`
  - `./ns3 run 'scratch/ub-quick-example scratch/2nodes_single-tp'`
- When the user needs current Unified Bus parameter/default guidance, initialize a runtime parameter catalog and reuse it:
  - this catalog is generated from `./ns3 run 'scratch/ub-quick-example scratch/2nodes_single-tp --PrintTypeIds'`
  - query per-component attributes via `--ClassName=...`
  - query Unified Bus globals via `--PrintUbGlobals`
  - treat the generated project parameter catalog as the current baseline until it is refreshed
  - generate `network_attribute.txt` as a full resolved snapshot from that catalog, not as a sample-case inheritance patch
- Do not rely on a hand-written static Unified Bus parameter table.
- Startup status belongs in the readiness/status part of `experiment-spec.md`, not as a hidden side channel.

## Supported `v1` Topology Families

This section defines the supported `v1` topology families.

- `ring`
- `full-mesh`
- `nd-full-mesh`
- `clos-spine-leaf`
- `clos-fat-tree`
- old case as bounded reference

If the user asks for something outside this surface, do not invent support. Ask them to restate it as one supported family or provide an old case reference.

## Supported `v1` Workload Modes

This section defines the supported `v1` workload modes.

- `ar_ring`
- `ar_nhr`
- `ar_rhd`
- `a2a_pairwise`
- `a2a_scatter`
- reference `traffic.csv`

If the user asks for something outside this surface, keep the conversation in clarification mode until it is mapped into one supported mode or anchored by a reference file.

## Decision Style

Use soft bounded options when guidance helps. Present them as `1/2/3/4` choices:

- `1:`
- `2:`
- `3:`
- `4: user free input`

Rules:

- only show the three most relevant templated choices
- `4: user free input` must remain available
- recommendations stay soft
- the user must still decide
- choices should narrow one current blocking decision, not reopen the whole intake

## Old Case Rule

- Old case input is a bounded reference, not an in-place edit path.
- Read only:
  - `network_attribute.txt`
  - `node.csv`
  - `topology.csv`
  - `routing_table.csv`
  - `transport_channel.csv`
  - `traffic.csv`
- Summarize what the old case says, then ask what to keep and what to change.

## Trace / Debug Rule

- `trace/debug` is part of `v1`, but only as an overlay on `network_attribute.txt`.
- The default recommendation is balanced mode.
- Do not invent a separate trace/debug output file.
- When discussing `minimal`, `balanced`, or `detailed`, read `references/domain/trace-observability.md` first.

## Domain Routing Rules

- When discussing `transport_channel.mode`, read `references/domain/transport-channel-modes.md` first.
- When discussing throughput evidence, line-rate questions, or whether a result is “close to line rate,” read `references/domain/throughput-evidence.md` first.

## Generation Gate

- Before generation, summarize:
  - what the user explicitly chose
  - what the agent filled in to make generation concrete
- summarize any startup facts that affect whether generation or execution is currently possible
- Case generation requires explicit final user approval.
- Without that approval, stay in clarification mode.
- If the user says “generate now” before the gate is satisfied, do not start commands, directory creation, file generation, or backend inspection.
- In that case, reply in natural language with:
  - the current bound experiment facts
  - the smallest remaining blocking decision
  - one bounded `1/2/3/4` choice set when helpful

## Checker Rule

- The checker is format-only.
- It may validate file presence and file shape.
- It is not a performance judgment.

## References

- For topology choice wording: read `references/topology-options.md`
- For workload choice wording: read `references/workload-options.md`
- For spec-writing and confirmation rules: read `references/spec-rules.md`
