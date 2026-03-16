---
name: openusim-analyze-results
description: Use when simulation outputs already exist and the user wants to interpret results against the original experiment goal and investigate likely causes.
---

# OpenUSim Analyze Results

## Overview

Interpret completed run outputs against the original experiment goal.
Make claims only to the extent supported by output files, case inputs, and repo code.

## When to Use

- The simulation has completed and outputs already exist.
- The user wants to know whether the results match the intended experiment goal.
- The user wants help investigating likely causes for an unexpected outcome.

Do not use this skill to initialize the repo or to define a new experiment from scratch.

## The Process

1. Read the current `experiment-spec.md`.
2. Inspect the relevant case inputs and output artifacts.
3. Choose the matching knowledge card before answering specialized domain questions.
4. Compare the observed evidence against the original experiment goal.
5. State what the evidence supports, what it does not support, and what remains uncertain.
6. If another iteration is needed, hand the next bounded decision back to planning.

## Stop And Ask

- The necessary output artifacts do not exist.
- The requested conclusion depends on evidence the repo does not currently produce.
- The user is actually changing the experiment definition rather than asking for result interpretation.

## Handover

Stay in this skill when:

- the user is still asking for interpretation of the current run
- the next question is still about result evidence or likely causes

Hand off to `openusim-plan-experiment` when:

- the user wants to change the experiment for another iteration
- the current analysis identifies a new bounded decision for the next run

Before handoff, record in `experiment-spec.md`:

- the most important findings that affect the next iteration
- the specific result gaps or hypotheses that motivated the next change

## Integration

- Called by: `openusim-run-experiment`, direct requests about existing outputs
- Hands off to: `openusim-plan-experiment`
- Required references:
  - `../openusim-references/throughput-evidence.md`
  - `../openusim-references/trace-observability.md`
  - `../openusim-references/transport-channel-modes.md`

## References

- `../openusim-references/throughput-evidence.md`
- `../openusim-references/trace-observability.md`
- `../openusim-references/transport-channel-modes.md`

## Common Mistakes

- Claiming line-rate conclusions without stating both the metric and the comparison target.
- Confusing runtime overhead from trace settings with simulated network semantics.
- Treating missing evidence as proof.
- Changing the experiment definition inside the analysis phase instead of handing back to planning.
