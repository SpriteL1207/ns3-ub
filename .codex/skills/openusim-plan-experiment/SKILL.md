---
name: openusim-plan-experiment
description: Use when the user wants to define one OpenUSim experiment and converge on a runnable experiment specification before any generation or execution.
---

# OpenUSim Plan Experiment

## Overview

Turn one natural-language OpenUSim goal into one stable `experiment-spec.md`.
Only solve one blocking planning slot per turn.

## When to Use

- The user wants to design a new experiment.
- The user wants to refine an old case into a new experiment.
- The user asks to generate or run, but the experiment definition is still incomplete.

Do not use this skill to verify repo startup or to interpret completed run results.

## The Process

1. Bind the user-provided facts before asking for more input.
2. Classify the first meaningful request as `broad`, `semi-specified`, or `reference-based`.
3. Resolve the first unstable planning slot in this order:
   - `goal`
   - `topology`
   - `workload`
   - `network_profile`
   - `observability`
   - `approval_ready`
4. Ask only one smallest blocking question per turn.
5. Use bounded `1/2/3/4` choices only when they help the user decide.
6. **When all slots are resolved and user approves generation/execution:**
   a. **Use Write tool to create `experiment-spec.md`** (template: `../openusim-references/spec-rules.md`)
   b. **Verify spec completeness** (use Read tool, check all required sections exist)
   c. **Announce handoff readiness** ("Spec written at `experiment-spec.md`, ready to hand off to run stage")

## Stop And Ask

- The requested topology or workload cannot be mapped to a bounded repo-supported path.
- The user asks to generate or run before the planning gate is satisfied.
- Startup facts are missing and block execution decisions.
- Repo startup facts have not been verified for this session and the user is approaching generation approval.

## Handover

Stay in this skill when:

- the experiment goal is still ambiguous
- topology or workload is still unresolved
- the user has not explicitly approved generation or execution

Hand off to `openusim-run-experiment` when:

- Step 6 of The Process is complete (spec written and verified)
- The experiment goal is stable
- Topology, workload, network parameters, and observability are concrete enough to run
- The user explicitly approved generation or execution

Before handoff, ensure `experiment-spec.md` exists on disk with all required sections from `../openusim-references/spec-rules.md`. The file must contain:

- the confirmed experiment goal
- the chosen topology path
- the chosen workload path
- the chosen network parameter overrides
- the chosen observability mode
- any assumptions needed to make execution concrete

Return to `openusim-welcome` when:

- repo startup facts are missing and block execution

## Integration

- Called by: user requests for experiment design, `openusim-welcome`, `openusim-analyze-results`
- Hands off to: `openusim-run-experiment`
- May return to: `openusim-welcome`
- Required references:
  - `../openusim-references/topology-options.md`
  - `../openusim-references/workload-options.md`
  - `../openusim-references/spec-rules.md`
  - `../openusim-references/spec-to-toolchain.md`

## References

- `../openusim-references/topology-options.md`
- `../openusim-references/workload-options.md`
- `../openusim-references/spec-rules.md`
- `../openusim-references/spec-to-toolchain.md`

## Common Mistakes

- Ignoring already-provided facts and falling back to a generic intake.
- Asking a full questionnaire when one missing decision is enough.
- Treating runtime switches such as `--mtp-threads` as core experiment-definition slots.
- Starting generation or backend inspection before the planning gate is satisfied.
