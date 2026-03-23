---
name: openusim-run-experiment
description: Use when the experiment specification is approved and the user wants to generate a case, run the simulation, and monitor execution status or explicit errors.
---

# OpenUSim Run Experiment

## Overview

Use repo-native tools to turn a stable `experiment-spec.md` into a runnable case and a concrete run.
Keep custom Python limited to `network_attribute.txt` writing and a light case-file gate.

## When to Use

- The experiment specification is approved for generation or execution.
- The user wants to build a runnable case from the planned experiment.
- The user wants to run the case and understand explicit execution failures.

Do not use this skill to define experiment intent or to perform final result interpretation.

## Input from Plan Stage

- `{case_dir}`: Full path to case directory under `scratch/` (e.g., `scratch/20260322-clos-32hosts-bw-test/`)
- This directory contains `experiment-spec.md` created by `openusim-plan-experiment`

All generation and execution happen within `{case_dir}/`.

## The Process

1. **Verify prerequisites:**
   a. **Accept `{case_dir}` from plan stage** (path to case directory under `scratch/`)
   b. **Read `{case_dir}/experiment-spec.md`** (must exist, use Read tool)
   c. **Check spec completeness:** verify all required sections exist (Goal, Topology, Workload, Network Overrides, Observability)
   d. **If spec is incomplete:** return to `openusim-plan-experiment`

2. Parse spec and extract execution parameters.
3. **Generate topology script in `{case_dir}/`:**
   a. Read topology family and parameters from spec
   b. Select code template from `../openusim-references/topology-options.md` Generation Patterns
   c. Generate `{case_dir}/generate_topology.py` by substituting parameters
   d. Run: `python3 {case_dir}/generate_topology.py`
   e. Verify outputs: `{case_dir}/node.csv`, `{case_dir}/topology.csv`, `{case_dir}/routing_table.csv`, `{case_dir}/transport_channel.csv`
4. Generate `traffic.csv` through `scratch/ns-3-ub-tools/traffic_maker/build_traffic.py` when the workload is not a reference file.
5. Write a full `network_attribute.txt` snapshot through the thin query-based writer.
6. Run the light case-file gate before execution.
7. Run `./ns3` with the chosen case and monitor explicit errors.
8. Record only durable execution facts in `experiment-spec.md`.

## Stop And Ask

- **The experiment specification does not exist or is incomplete** — return to plan stage.
- **Repo startup facts block execution** — return to welcome stage.
- The topology family in the spec has no mapped code template in `../openusim-references/topology-options.md` — ask the user to provide a custom script or restate as a supported family.
- Existing repo tools cannot express the requested case without a new bounded decision.

## Handover

Stay in this skill when:

- the case is still being generated
- the simulation has not completed
- the current failure is an explicit execution problem

Hand off to `openusim-analyze-results` when:

- the simulation completed and output artifacts exist
- the simulation failed or stalled, and there are console messages, partial outputs, or error logs to interpret
- the user wants interpretation rather than more execution retries

Before handoff, record in `experiment-spec.md`:

- the actual case path
- the exact run command or runtime switches that materially affect interpretation
- the presence of key output artifacts
- explicit execution failures that remain unresolved

Return to `openusim-welcome` when:

- repo startup facts are missing and block execution

Return to `openusim-plan-experiment` when:

- the experiment specification is incomplete or contradictory
- the user changes the experiment definition instead of retrying execution

## Integration

- Called by: `openusim-plan-experiment`
- Hands off to: `openusim-analyze-results`
- May return to: `openusim-welcome`, `openusim-plan-experiment`
- Required references:
  - `README.md`
  - `QUICK_START.md`
  - `scratch/README.md`
  - `scratch/ns-3-ub-tools/README.md`
  - `../openusim-references/spec-to-toolchain.md`
  - `../openusim-references/topology-options.md`
  - `../openusim-references/workload-options.md`
  - `../openusim-references/spec-rules.md`
  - `../openusim-references/transport-channel-modes.md`

## References

- `scratch/ns-3-ub-tools/net_sim_builder.py`
- `scratch/ns-3-ub-tools/user_topo_2layer_clos.py`
- `scratch/ns-3-ub-tools/user_topo_4x4_2DFM.py`
- `scratch/ns-3-ub-tools/traffic_maker/build_traffic.py`
- `../openusim-references/spec-to-toolchain.md`
- `../openusim-references/topology-options.md`
- `../openusim-references/workload-options.md`
- `../openusim-references/spec-rules.md`
- `../openusim-references/transport-channel-modes.md`

## Common Mistakes

- Copying or modifying `user_topo_*.py` scripts instead of generating new scripts from code templates.
- Treating `--mtp-threads` as part of experiment intent instead of runtime execution.
- Turning the case checker into a heavy semantic validator.
- Hiding explicit execution errors instead of surfacing them and using them to drive the next decision.
- Stating default parameter values (bandwidth, delay, etc.) from memory instead of querying the runtime catalog via `load_or_build_parameter_catalog()`. Always use the catalog; do not recite static values.
- Proceeding with generation when `experiment-spec.md` does not exist or is incomplete.
