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

## The Process

1. Read the current `experiment-spec.md`.
2. Verify that startup facts do not block execution.
3. Generate topology files through `scratch/ns-3-ub-tools/net_sim_builder.py` and its example topology scripts when applicable.
4. Generate `traffic.csv` through `scratch/ns-3-ub-tools/traffic_maker/build_traffic.py` when the workload is not a reference file.
5. Write a full `network_attribute.txt` snapshot through the thin query-based writer.
6. Run the light case-file gate before execution.
7. Run `./ns3` with the chosen case and monitor explicit errors.
8. Record only durable execution facts in `experiment-spec.md`.

## Stop And Ask

- The experiment specification is still missing required run facts.
- Repo startup facts block execution.
- Existing repo tools cannot express the requested case without a new bounded decision.
- The run fails in a way that is not self-explanatory from the error output.

## Handover

Stay in this skill when:

- the case is still being generated
- the simulation has not completed
- the current failure is an explicit execution problem

Hand off to `openusim-analyze-results` when:

- the simulation completed and output artifacts exist
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

## References

- `scratch/ns-3-ub-tools/net_sim_builder.py`
- `scratch/ns-3-ub-tools/user_topo_2layer_clos.py`
- `scratch/ns-3-ub-tools/user_topo_4x4_2DFM.py`
- `scratch/ns-3-ub-tools/traffic_maker/build_traffic.py`

## Common Mistakes

- Reintroducing custom topology or workload generators instead of using repo-native tools.
- Treating `--mtp-threads` as part of experiment intent instead of runtime execution.
- Turning the case checker into a heavy semantic validator.
- Hiding explicit execution errors instead of surfacing them and using them to drive the next decision.
