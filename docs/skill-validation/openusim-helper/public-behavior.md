# OpenUSim Helper Public Behavior

This document states the plain-language behavior expected from `openusim-helper`.

## Public Surface

The user should see one skill only:

- `openusim-helper`

The skill should not expose separate public phases for intake, preparation, or execution in `v1`.

## User-Facing Reply Surface

The user-facing reply is not a policy summary.

The skill should keep internal control information internal. In normal user-visible replies, it should not restate:

- skill names
- routing decisions
- compliance statements
- repository or prompt policy

The visible reply should instead follow this shape:

- bind the facts already provided by the user
- state the current step in one short user-facing sentence
- ask one `smallest blocking question`
- offer bounded `1/2/3/4` choices only when they help

This avoids a generic intake template and keeps the conversation anchored to the user's actual input.

This structure should usually stay implicit rather than appearing as visible labels.

Avoid control labels such as:

- `已知事实`
- `当前步骤`
- `下一步`
- `Decision`

Prefer natural transitions such as:

- “你这个目标可以先按…来收口”
- “先把…定下来”
- “接下来只差…”

## First Turn

When the user starts with a broad request such as "I want to run an OpenUSim simulation," the first reply should stay light.

This is the `broad first-turn request` case.

The first reply should:

- explain that the workflow starts by clarifying the experiment
- say that goal, topology, workload, and main parameters will be fixed step by step
- ask one question about what the user wants to learn from the simulation

The first turn should not:

- inspect backend implementation details
- create a case directory
- generate files
- imply that a run has started
- expose internal workflow labels such as `existing_case`, `prepared_case`, or `goal-to-experiment`

In short: `do not generate immediately`.

The wording should stay user-facing. Use phrases such as `old case reference` or `new experiment case` instead of legacy internal labels.

## First-Turn Classification

The skill should distinguish among three first-turn shapes:

- `broad`
- `semi-specified`
- `reference-based`

Expected handling:

- In `broad`, ask what the user wants to learn from the simulation.
- In `semi-specified`, bind the user-provided facts first and ask only the next missing decision.
- In `reference-based`, summarize the known reference facts first and ask what to keep or change.
- If the user says `2-layer Clos`, `leaf-spine`, or `spine-leaf`, bind that directly as `clos-spine-leaf`.
- Only reopen `clos-spine-leaf` versus `clos-fat-tree` when the user explicitly says `fat-tree`, gives `k`, or the wording is genuinely ambiguous.

It should not ignore already-provided facts by falling back to a generic intake template.

## Working Spec

The working artifact is one current Markdown file:

- `experiment-spec.md`

The skill should treat it as the single truth source for the current experiment description.

The spec is not:

- a chat transcript
- a revision log
- a run history

The skill should write back only after a major step becomes stable enough to keep.

## Project Startup Behavior

The skill must not bluff about repository readiness.

Before claiming that it can use `ns-3-ub-tools`, generate runnable cases, or run a simulation, it should anchor itself in:

- `README.md`
- `QUICK_START.md`

It should do this by reading those docs and operating on the repo directly, not by requiring a separate bootstrap helper script.

It should check these concrete startup facts:

- `./ns3` launcher exists
- `scratch/ns-3-ub-tools/` exists
- `scratch/ns-3-ub-tools/requirements.txt` exists
- critical scripts exist:
  - `scratch/ns-3-ub-tools/net_sim_builder.py`
  - `scratch/ns-3-ub-tools/traffic_maker/build_traffic.py`
  - `scratch/ns-3-ub-tools/trace_analysis/parse_trace.py`
- `build/` and `cmake-cache/` exist before calling the repo built
- `scratch/2nodes_single-tp` exists before proposing the default Quick Start smoke run

If these facts are missing, the skill should say which startup prerequisites are missing.

When the user clearly wants to start or run the project, the skill should help perform the bounded Quick Start startup flow rather than only describing it:

- `git submodule update --init --recursive`
- `python3 -m pip install --user -r scratch/ns-3-ub-tools/requirements.txt`
- `./ns3 configure`
- `./ns3 build`
- `./ns3 run 'scratch/ub-quick-example scratch/2nodes_single-tp'`

When current Unified Bus parameter/default guidance is needed, the helper should initialize and reuse a runtime parameter catalog instead of reciting a fixed table:

- runtime source: `./ns3 run 'scratch/ub-quick-example scratch/2nodes_single-tp --PrintTypeIds'`
- runtime source: `--ClassName=...`
- runtime source: `--PrintUbGlobals`
- treat the generated project parameter catalog as the current baseline until it is refreshed
- generate `network_attribute.txt` as a full resolved snapshot from that catalog, not as an inherited sample diff
- do not use a hand-written static Unified Bus parameter table

Startup status should be surfaced as part of readiness/status, not hidden behind undocumented assumptions.

## Domain Knowledge Behavior

The skill should not improvise specialized OpenUSim claims from generic intuition when a repo-local knowledge card exists.

For `trace/debug` semantics, it should read:

- ` .codex/skills/openusim-helper/references/domain/trace-observability.md `

Expected effect:

- prefer wording about observability, runtime cost, and post-processing cost
- avoid claiming that `detailed` trace itself lowers simulated throughput
- avoid treating stronger tracing as a network-semantic performance regression

For `transport_channel.mode` semantics, it should read:

- ` .codex/skills/openusim-helper/references/domain/transport-channel-modes.md `

Expected effect:

- avoid claiming that `transport_channel.csv` is always mandatory
- avoid treating missing `transport_channel.csv` as a defect when the mode is `on_demand`

For throughput evidence and line-rate interpretation, it should read:

- ` .codex/skills/openusim-helper/references/domain/throughput-evidence.md `

Expected effect:

- distinguish per-port throughput from per-task throughput
- avoid claiming line-rate conclusions without naming the metric source and comparison target

## Decision Prompts

When the user needs guidance, prompts should stay soft and bounded:

- `1`
- `2`
- `3`
- `4`

Rules:

- show only the three most relevant templated options
- reserve `4` for user free input or a hybrid answer
- keep recommendations soft
- require the user to explicitly choose or restate
- use options only to narrow one current blocking decision

## Question Granularity

Each turn should ask only the `smallest blocking question`.

This means the skill should:

- avoid asking for a full schema when one missing choice is enough
- avoid re-asking already stable facts
- move from bound facts to the next unresolved decision with minimal drift
- keep the wording conversational instead of sounding like a workflow dashboard

## Supported V1 Surface

`v1` is intentionally narrow.

Supported topology families:

- `ring`
- `full-mesh`
- `nd-full-mesh`
- `clos-spine-leaf`
- `clos-fat-tree`
- old case as bounded reference

Supported workload expressions:

- `ar_ring`
- `ar_nhr`
- `ar_rhd`
- `a2a_pairwise`
- `a2a_scatter`
- reference `traffic.csv`

If the user asks for a custom topology or workload outside this surface, the skill should not invent support. It should either:

- map the request clearly into one supported `v1` option
- or ask the user to anchor it with an old case or reference `traffic.csv`

## Old Case Reference

An old case is allowed only as a bounded reference.

The skill may read these files:

- `network_attribute.txt`
- `node.csv`
- `topology.csv`
- `routing_table.csv`
- `transport_channel.csv`
- `traffic.csv`

The skill should summarize the old case in four short parts:

- topology
- workload
- parameters
- missing information

After that, it must ask the user what to keep and what to change. The new experiment still gets a new spec and a new output directory.

## Generation Gate

Case generation must stop behind an explicit final approval step with explicit user approval.

Before generation, the skill should:

- summarize the current spec
- state any agent-supplied defaults used only to complete generation inputs
- confirm trace/debug choice
- ask the user to explicitly approve case generation

Without that approval, the skill should remain in clarification mode.

If the user says `generate now` or equivalent before the gate is satisfied, the skill should still remain in clarification mode.

In that situation it should not:

- start commands
- inspect backend scripts or code paths just because the user asked to generate
- create directories
- generate files

Instead it should:

- restate the currently bound experiment facts
- name the smallest remaining blocking decision
- ask only that one question, optionally with bounded `1/2/3/4` options

## Directory Creation

The experiment directory should be created only after the goal is stable enough to support the first durable summary.

The intended path shape is:

```text
scratch/openusim-generated/<yyyy-mm-dd-semantic-short-name>/
```

The skill should not create a new directory for every exploratory turn.

## Checker Behavior

The `v1` checker is format-only.

It may validate:

- required file presence
- file shape
- cross-file format compatibility

It must not claim:

- the simulation will fail for performance reasons
- the parameters are definitely bad
- the result quality is already known

This checker is `format-only` and `not a performance judgment`.

If a small mechanical issue is repairable without changing intent, the skill may do one bounded repair pass and report what changed. If the issue is not a small format repair, it should surface the factual problem to the user.
