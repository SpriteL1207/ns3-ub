# Repo Instructions

This repository standardizes on four repo-local OpenUSim stage skills:

- `openusim-welcome`
- `openusim-plan-experiment`
- `openusim-run-experiment`
- `openusim-analyze-results`

All four ship inside this repository under ` .codex/skills/ ` and are maintained with the main repo.

## Skill Routing

When the user wants help with Unified Bus / OpenUSim work in this repository, route by stage:

- use `openusim-welcome` for repo initialization, readiness, and bounded Quick Start smoke runs
- use `openusim-plan-experiment` for experiment definition and clarification
- use `openusim-run-experiment` for case generation, execution, and explicit run errors
- use `openusim-analyze-results` for result interpretation and likely-cause analysis

Do not route the user into legacy `openusim-*` skills unless the user explicitly asks to inspect or modify that legacy multi-skill system.

## User-Facing Reply Surface

The user-visible reply is not a policy summary.

Keep internal control information internal:

- do not echo skill names, routing decisions, or compliance statements to the user
- do not restate repository policy, prompt policy, or skill policy unless the user explicitly asks how the system works
- do not fall back to meta wording such as “I will follow this skill” or “I will process this with the repo rules”

The user-facing reply should contain only:

- the experiment facts already bound from the user's message
- one short statement of the current experiment step
- one smallest blocking question for the next decision
- bounded `1/2/3/4` options only when they help the user decide

Keep this structure implicit rather than labeled.

- avoid visible control labels such as `已知事实`, `当前步骤`, `下一步`, or `Decision` in normal chat replies
- prefer natural connective phrasing such as “你这个目标可以先按…来收口”, “先把…定下来”, “接下来只差…”
- only switch to explicit headings when the user asks for a structured summary

## First Reply Rule

For a broad first-turn request such as “我想做一次 openusim 仿真”:

- stay in planning mode
- explain that the workflow will first define the experiment goal, topology, workload, and key parameters
- ask one question about what the user wants to learn from the simulation

The first reply must not:

- start running commands
- inspect backend scripts or code paths
- create directories or generate files
- present internal workflow labels such as `existing_case`, `prepared_case`, or `goal-to-experiment`

Use natural user-facing wording instead, for example:

- old case reference
- new experiment case
- current experiment description

## First-Turn Classification

Classify the user's first meaningful OpenUSim request into one of these shapes:

- `broad`: the user only says they want to run or design a simulation
- `semi-specified`: the user already gives some combination of goal, topology, workload, or key parameter intent
- `reference-based`: the user gives an old case path, `traffic.csv`, or another bounded reference artifact

Handling rules:

- for `broad`, ask what the user wants to learn from the simulation
- for `semi-specified`, bind the user-provided facts first, then ask only the most blocking unresolved decision
- for `reference-based`, summarize the known reference facts first, then ask what to keep or change
- if the user says `2-layer Clos`, `leaf-spine`, or `spine-leaf`, bind that directly as `clos-spine-leaf`
- only ask `clos-spine-leaf` vs `clos-fat-tree` when the user explicitly says `fat-tree`, gives `k`, or the wording is genuinely ambiguous

Do not ignore already-provided facts by falling back to a generic intake template.

## Question Granularity

Ask only the smallest blocking question.

This means:

- do not ask for a full form when one missing decision is enough to continue
- do not reopen already stable choices
- if topology is already clear, ask about workload or scale instead of restating the whole intake
- if a reference case is already given, ask about the intended delta instead of asking from scratch
- keep the wording conversational rather than sounding like a checklist

## Generation Gate

Only generate or run a case after:

- the goal is stable enough to summarize
- the user has confirmed the main topology/workload/parameter choices
- the user gives explicit approval for generation or execution

If the user asks to generate or run before this gate is satisfied:

- do not start commands
- do not inspect backend scripts or code paths just because the user said `generate`
- do not create directories
- do not generate files
- reply with the current bound facts and the one smallest remaining blocking question

## Project Startup Gate

Before claiming that the repo can generate cases, use `ns-3-ub-tools`, or run a simulation, establish startup facts from:

- `README.md`
- `QUICK_START.md`

Do this by reading the docs and operating on the repo directly. Do not depend on a separate bootstrap helper script for this gate.

Check at least these repo facts:

- `./ns3` launcher exists
- `scratch/ns-3-ub-tools/` exists
- `scratch/ns-3-ub-tools/requirements.txt` exists
- critical tool scripts exist:
  - `scratch/ns-3-ub-tools/net_sim_builder.py`
  - `scratch/ns-3-ub-tools/traffic_maker/build_traffic.py`
  - `scratch/ns-3-ub-tools/trace_analysis/parse_trace.py`
- build artifacts such as `build/` and `cmake-cache/` exist before claiming the repo is built
- the default smoke case path `scratch/2nodes_single-tp` exists before proposing the Quick Start smoke run

Do not pretend startup is complete if these facts are missing.

When the user clearly wants to start or run the project, help execute the bounded Quick Start flow instead of only describing it:

- `git submodule update --init --recursive`
- `python3 -m pip install --user -r scratch/ns-3-ub-tools/requirements.txt`
- `./ns3 configure`
- `./ns3 build`
- `./ns3 run 'scratch/ub-quick-example scratch/2nodes_single-tp'`

When the user needs current Unified Bus parameter/default guidance, initialize a runtime parameter catalog and reuse it instead of reciting a fixed table:

- this runtime catalog is backed by `./ns3 run 'scratch/ub-quick-example scratch/2nodes_single-tp --PrintTypeIds'`
- it queries per-component attributes via `--ClassName=...`
- it queries Unified Bus globals via `--PrintUbGlobals`
- treat the generated project parameter catalog as the current baseline until it is refreshed
- generate `network_attribute.txt` as a full resolved snapshot from that catalog, not as a patch inherited from a sample case
- do not rely on a hand-written static Unified Bus parameter table

## Domain Knowledge Routing

When the discussion turns to a specialized OpenUSim domain topic that is easy to misstate from generic intuition, read the matching repo-local knowledge card before answering.

Current required card:

- for `trace/debug` semantics and recommendations: ` .codex/skills/openusim-references/trace-observability.md `
- for transport-channel semantics: ` .codex/skills/openusim-references/transport-channel-modes.md `
- for throughput evidence and line-rate interpretation: ` .codex/skills/openusim-references/throughput-evidence.md `
- for spec-to-toolchain mapping: ` .codex/skills/openusim-references/spec-to-toolchain.md `

## Skill Reference Maintenance

When tool scripts under `scratch/ns-3-ub-tools/` change their interface (parameters, CLI arguments, constraints, output schema), the corresponding reference files under `.codex/skills/openusim-references/` must be updated in the same commit.

Affected mappings:

- `net_sim_builder.py` API → `topology-options.md`, `spec-to-toolchain.md`
- `build_traffic.py` CLI → `workload-options.md`, `spec-to-toolchain.md`
- `trace_analysis/*.py` → `throughput-evidence.md`, `spec-to-toolchain.md`
