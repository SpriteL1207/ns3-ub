# OpenUSim Stage Skill Migration Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the legacy monolithic repo-local OpenUSim helper with four stage-specific skills, keep only the minimal runtime Python needed for `network_attribute.txt` generation and light case checks, and leave the repository with one unified documentation surface.

**Architecture:** The repo owns four stage skills under `./.codex/skills/`: startup/readiness, experiment planning, experiment execution, and result analysis. Most behavior lives in `SKILL.md` plus small reference cards; agent reasoning should read repo docs and native tools directly instead of depending on new heavy helper scripts. Python remains only where direct machine conversion is still justified: querying current ns-3 attributes into a resolved `network_attribute.txt`, and performing a light case-file presence gate before a run.

**Tech Stack:** Markdown skill docs, Python 3 `unittest`, `./ns3` query flags, `scratch/ns-3-ub-tools/net_sim_builder.py`, `scratch/ns-3-ub-tools/traffic_maker/build_traffic.py`

---

## File Structure

### Active skill surface

- `./.codex/skills/openusim-welcome/SKILL.md`
  Startup/readiness stage; reads `README.md` and `QUICK_START.md`, establishes repo facts, and supports the bounded Quick Start smoke flow.
- `./.codex/skills/openusim-plan-experiment/SKILL.md`
  Planning stage; converges one experiment description into `experiment-spec.md`.
- `./.codex/skills/openusim-plan-experiment/references/topology-options.md`
  Maps user topology wording onto repo-supported topology builders and examples.
- `./.codex/skills/openusim-plan-experiment/references/workload-options.md`
  Maps user workload wording onto repo-supported traffic tooling.
- `./.codex/skills/openusim-plan-experiment/references/spec-rules.md`
  Defines what must be stable before planning hands off to execution.
- `./.codex/skills/openusim-run-experiment/SKILL.md`
  Execution stage; generates a case with repo-native tools, writes `network_attribute.txt`, runs the simulation, and surfaces explicit run failures.
- `./.codex/skills/openusim-analyze-results/SKILL.md`
  Analysis stage; interprets outputs against the original goal and hands bounded next-step decisions back to planning.
- `./.codex/skills/openusim-analyze-results/references/throughput-evidence.md`
  Safe throughput interpretation rules.
- `./.codex/skills/openusim-analyze-results/references/trace-observability.md`
  Safe wording for trace/debug observability tradeoffs.
- `./.codex/skills/openusim-analyze-results/references/transport-channel-modes.md`
  Semantics for explicit vs on-demand transport-channel generation.

### Minimal runtime code

- `./.codex/skills/openusim-run-experiment/scripts/openusim_run_experiment/network_attribute_writer.py`
  Queries current ns-3 attributes and globals, resolves overrides, and writes a full `network_attribute.txt`.
- `./.codex/skills/openusim-run-experiment/scripts/openusim_run_experiment/case_checker.py`
  Performs a light case-file presence gate only.

### Routing and verification surface

- `./AGENTS.md`
  Repo routing rules for the four stage skills and the project startup gate.
- `./README.md`
  High-level project overview plus repo-local OpenUSim skill entrypoint description.
- `./QUICK_START.md`
  Startup/build/run instructions plus repo-local OpenUSim skill hinting.
- `./tests/openusim_skills/test_skill_docs.py`
  Verifies the stage-skill surface, handover markers, and legacy-surface retirement.
- `./tests/openusim_skills/test_run_scripts.py`
  Verifies the thin runtime Python that remains.

## Chunk 1: Establish The Stage Skill Surface

### Task 1: Define the four stage skills

**Files:**
- Create: `./.codex/skills/openusim-welcome/SKILL.md`
- Create: `./.codex/skills/openusim-plan-experiment/SKILL.md`
- Create: `./.codex/skills/openusim-plan-experiment/references/topology-options.md`
- Create: `./.codex/skills/openusim-plan-experiment/references/workload-options.md`
- Create: `./.codex/skills/openusim-plan-experiment/references/spec-rules.md`
- Create: `./.codex/skills/openusim-run-experiment/SKILL.md`
- Create: `./.codex/skills/openusim-analyze-results/SKILL.md`
- Create: `./.codex/skills/openusim-analyze-results/references/throughput-evidence.md`
- Create: `./.codex/skills/openusim-analyze-results/references/trace-observability.md`
- Create: `./.codex/skills/openusim-analyze-results/references/transport-channel-modes.md`

- [ ] Write each `description` in trigger-only style.
- [ ] Keep workflow logic in the skill body, not in frontmatter.
- [ ] Add explicit `Handover` and `Integration` sections that match the stage boundaries.
- [ ] Keep planning state in `experiment-spec.md`; do not add hidden JSON session state.

### Task 2: Re-route the repository surface

**Files:**
- Modify: `./AGENTS.md`
- Modify: `./README.md`
- Modify: `./QUICK_START.md`

- [ ] Route initialization/readiness to `openusim-welcome`.
- [ ] Route experiment definition to `openusim-plan-experiment`.
- [ ] Route generation/execution to `openusim-run-experiment`.
- [ ] Route result interpretation to `openusim-analyze-results`.
- [ ] Remove user-facing references to the retired monolithic helper surface.

## Chunk 2: Keep Only The Runtime Python That Is Still Justified

### Task 3: Implement thin execution helpers

**Files:**
- Create: `./.codex/skills/openusim-run-experiment/scripts/openusim_run_experiment/__init__.py`
- Create: `./.codex/skills/openusim-run-experiment/scripts/openusim_run_experiment/network_attribute_writer.py`
- Create: `./.codex/skills/openusim-run-experiment/scripts/openusim_run_experiment/case_checker.py`

- [ ] Build `network_attribute_writer.py` on top of `./ns3` query capabilities instead of a hand-maintained static parameter table.
- [ ] Keep `case_checker.py` light: verify missing major files first, then let the agent inspect actual run errors instead of encoding a heavy rules engine.
- [ ] Do not reintroduce custom topology or workload writers; rely on `net_sim_builder.py`, `user_topo_2layer_clos.py`, `user_topo_4x4_2DFM.py`, and `traffic_maker/build_traffic.py`.

## Chunk 3: Remove Legacy Surface And Verify The Final State

### Task 4: Retire obsolete artifacts

**Files:**
- Delete: `./.codex/skills/<legacy-monolith>/...`
- Delete: `./tests/<legacy-helper-tests>/...`
- Delete: `./docs/plans/<obsolete-docs>.md`
- Delete: `./docs/skill-validation/<obsolete-helper-docs>/...`

- [ ] Delete the retired monolithic helper tree without compatibility wrappers.
- [ ] Delete helper-specific tests that no longer represent the new stage architecture.
- [ ] Delete obsolete design and validation docs that would pollute the new source of truth.
- [ ] Remove empty directories left behind by the retirement.

### Task 5: Verify the final repo state

**Files:**
- Create: `./tests/openusim_skills/__init__.py`
- Create: `./tests/openusim_skills/test_skill_docs.py`
- Create: `./tests/openusim_skills/test_run_scripts.py`

- [ ] Run: `python3 -m unittest tests.openusim_skills.test_skill_docs -v`
- [ ] Run: `python3 -m unittest tests.openusim_skills.test_run_scripts -v`
- [ ] Run: `python3 -m unittest tests.openusim_skills -v`
- [ ] Run a residual search for retired helper terms and confirm any remaining hits are intentional.
- [ ] Re-check `git status --short` before closing the task.

## Expected End State

- The repo exposes four stage-specific OpenUSim skills instead of one monolithic entrypoint.
- Startup, planning, execution, and analysis responsibilities are clearly separated.
- The only remaining Python helpers are the ones that still need machine conversion or lightweight checks.
- `README.md`, `QUICK_START.md`, and `AGENTS.md` all describe the same architecture.
- Obsolete helper docs, scripts, tests, and empty directories are gone.
