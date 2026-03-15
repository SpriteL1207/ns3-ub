# OpenUSim Helper Skill Validation

This directory records the public behavior contract for the repo-local `openusim-helper` skill.

`v1` exposes one user-facing skill only:

- `openusim-helper`

This skill is repo-local:

- it lives under ` .codex/skills/openusim-helper/ `
- it depends on the current repository state, including `./ns3`, `scratch/ub-quick-example`, and `scratch/ns-3-ub-tools/`
- it is not validated or packaged as an independent git submodule

The validation focus is narrow and documentation-first:

- broad first turns stay light and clarification-first
- one current `experiment-spec.md` is the only working spec
- topology and workload discussion stays inside the bounded `v1` surface
- option prompts use `1/2/3/4`, with `4` reserved for free input
- old cases are bounded references, not direct in-place edits
- case generation requires explicit final user approval
- the checker is format-only and must not claim simulation quality

This directory does not prove live runtime quality by itself. It defines what the skill must say and what boundaries it must preserve.
