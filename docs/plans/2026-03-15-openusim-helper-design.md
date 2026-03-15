# OpenUSim Helper Design

**Goal:** Define a new repo-local `openusim-helper` skill that turns a user's natural-language simulation goal into one current `experiment-spec.md` plus a runnable Unified Bus case directory for `ub-quick-example`.

**Status:** This document is the active design source for the repo-local `openusim-helper` shipped under ` .codex/skills/openusim-helper/ `. The old multi-skill and CLI-heavy OpenUSim direction is historical reference only and is not the shipped entrypoint.

## Problem Statement

The previous OpenUSim direction became too heavy for the user experience we actually want.

Three issues dominate:

1. The public surface fragmented into multiple skills, which forced the agent to expose internal workflow phases to the user.
2. The implementation drifted toward multi-stage orchestration, JSON artifacts, and CLI-heavy backend plumbing.
3. The skill tended to jump from a broad user intent straight into heavy reads or execution, instead of first helping the user define the experiment clearly.

The desired experience is simpler:

- the user talks to one skill
- the agent gradually clarifies the simulation goal
- one current Markdown spec becomes the single truth source
- deterministic helpers only write the final case files

## Design Principles

### 1. One user-facing skill

The user should only interact with one skill:

- `openusim-helper`

Internal stages such as goal clarification, topology drafting, workload drafting, and case generation must not appear as separate user-facing skills in `v1`.

### 2. Natural-language first

The user should describe intent in natural language. The agent is responsible for:

- asking follow-up questions
- summarizing current understanding
- identifying what information is still missing
- asking the user to explicitly confirm important decisions

The system must not force the user to fill a static schema or JSON artifact.

### 3. One current spec, not a chat transcript

The working artifact for each experiment is:

- `experiment-spec.md`

This document is:

- the current experiment description
- rewritten and cleaned up by the agent over time
- the single truth source for case generation inputs

It is not:

- a raw chat log
- a history ledger
- a run/analysis archive

### 4. The agent drives generation, not a Markdown parser

`experiment-spec.md` is the truth source, but `v1` does not require a script that directly parses free-form Markdown into every output file.

Instead:

- the agent reads and maintains the spec
- the agent decides whether enough information exists
- the agent calls a few deterministic Python helper functions with explicit arguments
- those helpers write case files

This keeps the spec natural-language friendly while avoiding a brittle full-document parser.

More precisely:

- spec sections `1` through `5` are the authoritative generation inputs
- spec sections `6` and `7` are derived status written by the agent
- derived status must never become an additional hidden input channel for generation

### 5. Do not statically judge simulation success

`v1` may do hard checks on file presence, schema shape, and format compatibility.

`v1` must not claim:

- the case cannot run because of inferred network risk
- the simulation result will be bad
- the parameter set is definitely wrong for performance reasons

If needed, the agent may mention uncertainty or missing information, but it should not turn static heuristics into hard prohibitions.

### 6. Historical OpenUSim work is reference, not migration target

Earlier OpenUSim multi-skill and CLI-heavy implementations may still be useful as historical reference for:

- terminology
- useful helper primitives
- existing file-writing logic
- existing validation ideas

But `v1` should not be blocked by:

- multi-skill compatibility
- dual-runtime compatibility
- keeping old CLI-centered orchestration as the main path
- migrating all previous tests before the new skill exists

## V1 Scope

`openusim-helper v1` supports one main workflow:

1. clarify a simulation goal
2. maintain one current `experiment-spec.md`
3. generate a new case directory that `ub-quick-example` can consume
4. run a light hard checker on generated files

`v1` does not include:

- run orchestration
- trace analysis workflows
- multi-round expert diagnosis loops
- keeping run history inside the spec
- full in-place editing of an existing case

## Supported Input Surface For V1

`v1` is intentionally not a general natural-language-to-topology compiler.

### Supported topology inputs

The agent may guide the user toward these built-in topology families:

- `ring`
- `full-mesh`
- `nd-full-mesh`
- `clos-spine-leaf`
- `clos-fat-tree`
- `old-case reference`

If the user asks for a more custom topology, `v1` should only proceed if the request can be cleanly reduced into one of the supported families or into a bounded old-case reference workflow.

Otherwise, the agent should say that the topology is not yet supported in `v1` and ask the user to:

- restate it as one of the supported families
- or provide an old case as a reference

### Supported workload inputs

The agent may guide the user toward these built-in workload modes:

- `ar_ring`
- `ar_nhr`
- `ar_rhd`
- `a2a_pairwise`
- `a2a_scatter`
- `reference traffic.csv`

If the user asks for a pattern such as `incast`, `hotspot`, or another custom flow shape, the agent may only continue if:

- it can map the request to one supported workload mode and explain that mapping
- or the user provides a reference `traffic.csv` or old case to anchor the workload

Otherwise, the agent should keep the request in discussion mode and ask the user to choose a supported workload expression for `v1`.

### Supported parameter inputs

The agent should only drive discussion around:

- `buffer/credit`
- `packet/cell`
- `routing/load-balance`
- `rate/timing`
- `trace/debug` as a later overlay choice

This means `ready for generation` in `v1` is not “the agent understood every possible network idea”; it is “the agent normalized the user’s intent into the supported topology, workload, parameter, and trace/debug input surface.”

## User Interaction Model

### First-turn behavior

If the user says something broad such as "我想用 openusim 做一次仿真", the skill should not start execution or inspect backend code.

The first response should only:

- explain that the workflow will first clarify the experiment
- say that the conversation will define goal, topology, workload, and parameters step by step
- ask one question about what the user wants to learn from the simulation

### Conversation sequence

The default interaction sequence is:

1. clarify the experiment goal
2. clarify topology, routing, and transport-channel intent
3. clarify workload intent
4. clarify only the most relevant network-parameter categories
5. confirm `trace/debug` options near generation time
6. ask for final approval to generate the case
7. generate files and run a light hard checker

The agent should speak naturally to the user, for example:

- "我们先把这次仿真想回答的问题说清楚"
- "接下来把拓扑和路由收口一下"
- "下面我帮你把流量模式定下来"

The agent should not expose internal wording like "Section 2" or "schema".

### Decision prompts

When the user would benefit from guidance, the skill should present narrow choices in this style:

- `1:`
- `2:`
- `3:`
- `4: 用户自行输入`

Rules:

- only show the three most relevant templated options
- always reserve option `4` for free input or hybrid modification
- recommendations are soft, not mandatory
- the user must explicitly decide

### Confirmation rule

For each major step, the agent should:

1. summarize current understanding in natural language
2. state what is still missing, if anything
3. ask the user to confirm or adjust

The agent should only ask for confirmation when the information is sufficient to support downstream file generation for that step.

## Experiment Directory Layout

The experiment directory should be created only after the goal is clear enough to write the first stable summary.

Path pattern:

```text
scratch/openusim-generated/<yyyy-mm-dd-semantic-short-name>/
```

Naming rules:

- the agent should generate a semantic short name
- it should try to avoid collisions by choosing a differentiating name first
- if there is still a collision, append `-v2`, `-v3`, and so on
- do not reuse an old experiment directory for a new experiment

The `v1` layout is:

```text
scratch/openusim-generated/<experiment-id>/
  experiment-spec.md
  case/
    network_attribute.txt
    node.csv
    topology.csv
    routing_table.csv
    transport_channel.csv
    traffic.csv
```

## Experiment Spec Structure

`experiment-spec.md` should be a stable working document with the following structure.

### 1. Goal

Describe:

- what question the simulation should answer
- what would count as success, failure, or a useful outcome
- what conclusion or evidence the user ultimately wants

### 2. Topology

Describe:

- one supported topology family, or an explicitly bounded old-case reference
- routing intent
- transport-channel intent

This part must eventually support generation of:

- `node.csv`
- `topology.csv`
- `routing_table.csv`
- `transport_channel.csv`

### 3. Workload

Describe:

- one supported workload mode, or an explicitly bounded reference `traffic.csv`
- offered load or qualitative load goal
- source/destination structure
- any timing or burst intent that matters for `traffic.csv`

This part must eventually support generation of:

- `traffic.csv`

### 4. Network Parameters

Describe the main simulation parameters that affect case generation, especially around:

- `buffer/credit`
- `packet/cell`
- `routing/load-balance`
- `rate/timing`

The agent should not dump every possible ns-3 parameter. It should ask only about the two or three categories most relevant to the current goal.

### 5. Trace / Debug Options

This is a separate small part, not mixed into main network parameters.

Default user-facing modes:

- `最小模式`
- `平衡模式`
- `详细模式`
- `用户自行指定`

The default recommendation is `平衡模式`.

The agent may make only small adjustments to the chosen mode and must explain them during final confirmation.

In `v1`, this section is not a separate runtime subsystem. It is a bounded overlay that maps into extra `network_attribute.txt` overrides.

Implications:

- `trace/debug` does not create another case file in `v1`
- its implementation authority lives inside `network_attribute_writer`
- if a requested trace/debug option cannot be expressed through supported attribute overrides, the agent should mark it unsupported in `v1` rather than inventing a new output path

### 6. Readiness / Status

This part is maintained by the agent and states:

- whether the spec is currently ready for case generation
- what information is still missing
- what still needs explicit user confirmation
- what the recommended next step is

This readiness judgment is only about information completeness and generation conditions, not about predicted simulation quality.

### 7. Case Output

This part records factual generation results, such as:

- case directory path
- which files were generated
- whether the current case matches the latest spec

It must not become a run-history or analysis-history section in `v1`.
It is derived bookkeeping only, not part of the generation input truth.

## Spec Writing Rules

### Write-back timing

The agent should not rewrite `experiment-spec.md` on every user turn.

It should write back only when:

- a major step has gained stable new information
- the user has confirmed that part
- generation results need to be recorded

### Current version only

`experiment-spec.md` should keep only the latest current truth.

Do not store:

- revision history
- prior alternatives
- old rejected plans

### Source marking

The spec should remain natural-language oriented, but it should lightly distinguish:

- user-confirmed decisions
- agent-supplied completion needed for case generation

This can be done with short natural phrases such as:

- `用户已确认：...`
- `Agent 为生成 case 暂定：...`

### What not to write yet

If the user supplies an old case as a reference, extracted information from that old case must not be written into the new `experiment-spec.md` until the user has explicitly said what to keep and what to change.

## Old Case Reference Behavior

`v1` may accept an old case path as reference input, but only in a bounded way.

The agent may read only:

- `network_attribute.txt`
- `node.csv`
- `topology.csv`
- `routing_table.csv`
- `transport_channel.csv`
- `traffic.csv`

The agent should then summarize the old case in four short parts:

- topology
- workload
- parameters
- currently missing information

After that, the agent must ask the user which parts to keep and which to change.

The new experiment still gets:

- a new `experiment-spec.md`
- a new experiment directory
- a new generated case

## Backend Shape For V1

The backend should stay minimal. The preferred shape is one new repo-local skill bundle:

```text
.codex/skills/openusim-helper/
  SKILL.md
  agents/openai.yaml
  references/
  scripts/
```

The scripts layer should expose a few small Python modules or functions, not one large CLI entrypoint.

Preferred minimal helper responsibilities:

1. `topology_writer`
   - writes `node.csv`
   - writes `topology.csv`
   - writes `routing_table.csv`
   - writes `transport_channel.csv`
2. `workload_writer`
   - writes `traffic.csv`
3. `network_attribute_writer`
   - writes `network_attribute.txt`
4. `case_assembler` / `case_checker`
   - ensures directory structure exists
   - records output facts
   - validates presence/format contracts

## Normalized Helper Contract

Before generation, the agent should normalize its current understanding into a small in-memory generation bundle.

The helper layer should not consume raw free-form Markdown. It should consume normalized Python data shaped approximately like this:

```python
generation_bundle = {
    "experiment": {
        "experiment_id": "...",
        "workspace_dir": "...",
        "case_dir": "...",
    },
    "topology_plan": {
        "mode": "template" | "old_case_reference",
        "family": "ring" | "full-mesh" | "nd-full-mesh" | "clos-spine-leaf" | "clos-fat-tree",
        "parameters": {...},
        "routing": {...},
        "transport_channel": {...},
        "user_confirmed": {...},
        "agent_completed": {...},
    },
    "workload_plan": {
        "mode": "template" | "reference_csv",
        "algorithm": "ar_ring" | "ar_nhr" | "ar_rhd" | "a2a_pairwise" | "a2a_scatter",
        "parameters": {...},
        "reference_csv": None,
        "user_confirmed": {...},
        "agent_completed": {...},
    },
    "network_attribute_plan": {
        "resolution_mode": "full-catalog-snapshot",
        "explicit_overrides": {...},
        "derived_overrides": {...},
    },
    "trace_debug_plan": {
        "mode": "minimal" | "balanced" | "detailed" | "custom",
        "attribute_overrides": {...},
    },
}
```

Rules:

- defaults are filled by the agent during normalization, not by the low-level file writers
- `network_attribute.txt` is emitted as a full resolved snapshot from the current project parameter catalog, not by inheriting a sample case
- each writer validates only its own input contract and output files
- `case_checker` validates cross-file presence and format compatibility
- `ready for generation` means every required normalized plan exists, has no unresolved required field, and has passed explicit user confirmation gates

This contract is intentionally internal and Python-native. It is not a persisted JSON artifact.

Design rules:

- prefer Python module functions over a central `cli.py`
- let the agent organize arguments in memory, then call helpers
- reuse `ns-3-ub-tools` primitives when they exist and match the need
- only add new helper code where a clear gap exists
- keep normalization responsibility above the writers, not inside them

## Generation And Checking Rules

The agent must not generate the final case immediately after collecting enough information.

It should first provide a final natural-language summary in two groups:

- what the user explicitly chose
- what the agent filled in to make case generation concrete

Only after the user explicitly says to generate should the skill write the case.

The actual readiness gate is:

- supported topology input chosen
- supported workload input chosen
- network parameters normalized enough for `network_attribute.txt`
- trace/debug overlay chosen
- no unresolved required field remains in the normalized generation bundle
- user has explicitly approved generation

After generation, the skill should run a light hard checker that verifies things like:

- required files exist
- expected fields exist
- file formats match the downstream contract

If the checker finds a small fixable issue, the agent may repair it once and re-run the checker.

If a fix changes experiment meaning or introduces a new assumption, the agent must ask the user before applying it.

## Reference Material Policy

The new skill may consult old material for targeted facts, but should not absorb the old system wholesale.

Use old material only for:

- existing file format knowledge
- reusable helper functions
- bounded validation rules
- terminology or option wording that still matches the new design

Do not treat the following as `v1` implementation obligations:

- preserving old multi-skill public behavior
- wrapping all old entrypoints
- migrating all old validation tests first
- preserving old JSON artifact flows

## Non-Goals

`v1` does not try to:

- redesign Unified Bus semantics
- predict performance outcomes from static reasoning
- support the full run / trace / analysis loop
- fully edit old experiments in place
- turn the spec into a machine-readable schema

## Future Extension Points

If `v1` works well, later versions may add:

- a second-stage run skill that consumes generated cases
- stronger old-case import support
- richer trace/debug presets
- optional analysis notes outside the main `experiment-spec.md`

These should be added as later scoped projects, not folded into `v1`.
