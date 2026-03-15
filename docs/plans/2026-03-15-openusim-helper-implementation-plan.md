# OpenUSim Helper Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a new repo-local `openusim-helper` skill that helps the user clarify a Unified Bus experiment, maintains one `experiment-spec.md`, and generates a new runnable case directory plus light hard-check results.

**Architecture:** Create `openusim-helper` as a repo-local skill under `./.codex/skills/`. Keep the skill conversation-first and spec-first; use a few small Python modules for workspace naming, spec writing, file generation, and hard checking. Pull runtime parameter facts from the current project via `./ns3` query entrypoints and reuse the real `scratch/ns-3-ub-tools/` backends directly instead of carrying a parallel compatibility layer.

**Tech Stack:** Markdown skill docs, YAML skill metadata, Python 3 standard library, repo-local helper modules under `./.codex/skills/openusim-helper/scripts/`, runtime `./ns3` attribute queries, direct `scratch/ns-3-ub-tools/` reuse, `unittest`

---

## Scope Guardrails

This plan intentionally does **not** include:

- migrating or preserving the old `openusim-*` public skill surface
- turning `tools/openusim/` and `skills/openusim/` into a new compatibility architecture
- keeping JSON artifacts or the old orchestration flow
- implementing run / trace / analysis workflows in `v1`
- deleting old files unless they directly conflict with the new skill

Old material is reference only. The success condition for this plan is a usable `openusim-helper`, with cleanup work treated as a later follow-up rather than a blocker.

## File Structure

### New user-facing skill bundle

- Create: `./.codex/skills/openusim-helper/SKILL.md`
- Create: `./.codex/skills/openusim-helper/agents/openai.yaml`
- Create: `./.codex/skills/openusim-helper/references/topology-options.md`
- Create: `./.codex/skills/openusim-helper/references/workload-options.md`
- Create: `./.codex/skills/openusim-helper/references/spec-rules.md`

### New deterministic helper modules

- Create: `./.codex/skills/openusim-helper/scripts/openusim_helper/__init__.py`
- Create: `./.codex/skills/openusim-helper/scripts/openusim_helper/contracts.py`
- Create: `./.codex/skills/openusim-helper/scripts/openusim_helper/workspace.py`
- Create: `./.codex/skills/openusim-helper/scripts/openusim_helper/spec.py`
- Create: `./.codex/skills/openusim-helper/scripts/openusim_helper/topology_writer.py`
- Create: `./.codex/skills/openusim-helper/scripts/openusim_helper/workload_writer.py`
- Create: `./.codex/skills/openusim-helper/scripts/openusim_helper/network_attribute_writer.py`
- Create: `./.codex/skills/openusim-helper/scripts/openusim_helper/case_checker.py`

### New validation docs

- Create: `docs/skill-validation/openusim-helper/README.md`
- Create: `docs/skill-validation/openusim-helper/public-behavior.md`
- Create: `docs/skill-validation/openusim-helper/generated-case-contract.md`

### New tests

- Create: `tests/openusim_helper/__init__.py`
- Create: `tests/openusim_helper/test_skill_docs.py`
- Create: `tests/openusim_helper/test_contracts.py`
- Create: `tests/openusim_helper/test_workspace.py`
- Create: `tests/openusim_helper/test_spec.py`
- Create: `tests/openusim_helper/test_writers.py`
- Create: `tests/openusim_helper/test_case_checker.py`
- Create: `tests/openusim_helper/test_end_to_end.py`

## Reference Inputs To Reuse Carefully

During implementation, consult these existing files only when they directly reduce duplication:

- `scratch/README.md`
- `scratch/ns-3-ub-tools/README.md`
- `scratch/ns-3-ub-tools/net_sim_builder.py`
- `scratch/ns-3-ub-tools/traffic_maker/build_traffic.py`
- `scratch/ub-quick-example.cc`
- `src/unified-bus/model/ub-utils.cc`

Do not mirror their public surface. Pull only the smallest useful logic or calling pattern.

## Chunk 1: Public Skill Surface

### Task 1: Create the repo-local skill shell

**Files:**
- Create: `./.codex/skills/openusim-helper/SKILL.md`
- Create: `./.codex/skills/openusim-helper/agents/openai.yaml`
- Create: `./.codex/skills/openusim-helper/references/topology-options.md`
- Create: `./.codex/skills/openusim-helper/references/workload-options.md`
- Create: `./.codex/skills/openusim-helper/references/spec-rules.md`
- Test: `tests/openusim_helper/test_skill_docs.py`

- [ ] **Step 1: Write a doc-level validation test for the new skill surface**

Add assertions that the new skill docs mention:
- one public skill only: `openusim-helper`
- first-turn behavior must ask about the experiment goal before generating anything
- one current `experiment-spec.md`
- explicit final user approval before case generation
- `1/2/3/4` option style with `4` reserved for free input
- bounded `v1` topology and workload support instead of open-ended natural-language compilation

- [ ] **Step 2: Run the doc-level test to verify it fails**

Run: `python3 -m unittest tests.openusim_helper.test_skill_docs -v`
Expected: FAIL because the new skill bundle does not exist yet.

- [ ] **Step 3: Write the skill shell**

Create `SKILL.md` that encodes:
- natural-language-first conversation
- stepwise flow: goal -> topology -> workload -> parameters -> trace/debug -> readiness -> generation
- write-back timing rules for `experiment-spec.md`
- old-case reference behavior
- final generation gate
- supported topology families and supported workload modes for `v1`
- what to do when the user asks for something beyond `v1`

Create `agents/openai.yaml` so repo-local Codex can discover the skill.

Keep references concise:
- `topology-options.md`: short topology option summaries and when they fit
- `workload-options.md`: short workload pattern summaries and when they fit
- `spec-rules.md`: spec-writing and confirmation rules

- [ ] **Step 4: Run the doc-level test to verify it passes**

Run: `python3 -m unittest tests.openusim_helper.test_skill_docs -v`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add .codex/skills/openusim-helper tests/openusim_helper/test_skill_docs.py
git commit -m "feat: add openusim-helper repo-local skill shell"
```

### Task 2: Add public behavior validation notes for the new skill

**Files:**
- Create: `docs/skill-validation/openusim-helper/README.md`
- Create: `docs/skill-validation/openusim-helper/public-behavior.md`
- Modify: `tests/openusim_helper/test_skill_docs.py`

- [ ] **Step 1: Extend the validation test with behavior markers**

Require validation docs to describe:
- broad first-turn user intent should not trigger heavy execution
- the experiment directory is created only after the goal is stable
- final case generation requires explicit user approval
- hard checking is format-only and not a performance judgment

- [ ] **Step 2: Run the test to verify it fails**

Run: `python3 -m unittest tests.openusim_helper.test_skill_docs -v`
Expected: FAIL because validation docs do not exist yet.

- [ ] **Step 3: Write the validation docs**

Document the expected interaction pressure cases in `public-behavior.md`, especially:
- broad first turn
- old case as bounded reference input
- final generation confirmation
- checker failure and one-pass auto-repair for small issues

- [ ] **Step 4: Run the test to verify it passes**

Run: `python3 -m unittest tests.openusim_helper.test_skill_docs -v`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add docs/skill-validation/openusim-helper tests/openusim_helper/test_skill_docs.py
git commit -m "docs: validate openusim-helper public behavior"
```

## Chunk 2: Normalization, Workspace And Spec Core

### Task 3: Implement the normalized generation contract and ready gate

**Files:**
- Create: `./.codex/skills/openusim-helper/scripts/openusim_helper/contracts.py`
- Create: `tests/openusim_helper/test_contracts.py`

- [ ] **Step 1: Write focused tests for the internal contract**

Cover at least:
- supported topology families are explicitly enumerated
- supported workload algorithms are explicitly enumerated
- normalized plans separate user-confirmed fields from agent-completed fields
- `trace/debug` is represented as a bounded attribute overlay, not a separate output subsystem
- `ready for generation` fails when any required normalized field is unresolved

- [ ] **Step 2: Run the test to verify it fails**

Run: `python3 -m unittest tests.openusim_helper.test_contracts -v`
Expected: FAIL because `contracts.py` does not exist yet.

- [ ] **Step 3: Write the minimal contracts module**

Implement narrow helpers such as:

```python
SUPPORTED_TOPOLOGY_FAMILIES = (...)
SUPPORTED_WORKLOAD_ALGOS = (...)

def make_topology_plan(...) -> dict: ...
def make_workload_plan(...) -> dict: ...
def make_network_attribute_plan(...) -> dict: ...
def make_trace_debug_plan(...) -> dict: ...
def generation_ready(bundle: dict) -> tuple[bool, list[str]]: ...
```

Do not persist this contract to JSON. Keep it as internal Python data only.

- [ ] **Step 4: Run the test to verify it passes**

Run: `python3 -m unittest tests.openusim_helper.test_contracts -v`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add .codex/skills/openusim-helper/scripts/openusim_helper/contracts.py tests/openusim_helper/test_contracts.py
git commit -m "feat: add openusim-helper generation contracts"
```

### Task 4: Implement experiment workspace naming and directory creation

**Files:**
- Create: `./.codex/skills/openusim-helper/scripts/openusim_helper/__init__.py`
- Create: `./.codex/skills/openusim-helper/scripts/openusim_helper/workspace.py`
- Create: `tests/openusim_helper/test_workspace.py`

- [ ] **Step 1: Write focused tests for workspace creation**

Cover at least:
- semantic slug creation from a goal summary
- collision handling that tries a differentiating name first, then falls back to `-v2`
- directory layout creation under `scratch/openusim-generated/<experiment-id>/`
- no directory creation before a stable goal is available

- [ ] **Step 2: Run the test to verify it fails**

Run: `python3 -m unittest tests.openusim_helper.test_workspace -v`
Expected: FAIL because `workspace.py` does not exist yet.

- [ ] **Step 3: Write the minimal workspace module**

Implement functions similar to:

```python
def make_experiment_slug(date_text: str, goal_summary: str, existing_names: set[str]) -> str: ...
def create_experiment_workspace(repo_root: Path, experiment_id: str) -> Path: ...
def case_dir_for_workspace(workspace_dir: Path) -> Path: ...
```

The module should create:
- `experiment-spec.md`
- `case/`

only when called with a stable experiment id.

- [ ] **Step 4: Run the test to verify it passes**

Run: `python3 -m unittest tests.openusim_helper.test_workspace -v`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add .codex/skills/openusim-helper/scripts/openusim_helper/__init__.py .codex/skills/openusim-helper/scripts/openusim_helper/workspace.py tests/openusim_helper/test_workspace.py
git commit -m "feat: add openusim-helper workspace creation"
```

### Task 5: Implement spec rendering and update rules

**Files:**
- Create: `./.codex/skills/openusim-helper/scripts/openusim_helper/spec.py`
- Create: `tests/openusim_helper/test_spec.py`

- [ ] **Step 1: Write focused tests for spec rendering**

Cover at least:
- rendering the seven-part structure
- writing only current truth, not history
- sections `1` through `5` act as generation inputs while sections `6` and `7` remain derived status
- preserving the separation between user-confirmed content and agent-completed content
- not writing old-case extracted data into the new spec before user confirmation
- updating readiness and case-output sections independently of the main sections

- [ ] **Step 2: Run the test to verify it fails**

Run: `python3 -m unittest tests.openusim_helper.test_spec -v`
Expected: FAIL because `spec.py` does not exist yet.

- [ ] **Step 3: Write the minimal spec module**

Implement focused helpers such as:

```python
def render_experiment_spec(model: dict) -> str: ...
def write_experiment_spec(spec_path: Path, model: dict) -> None: ...
def summarize_old_case_reference(extracted: dict) -> str: ...
```

The model can stay as an internal Python dict. Do not introduce a persisted JSON artifact.

- [ ] **Step 4: Run the test to verify it passes**

Run: `python3 -m unittest tests.openusim_helper.test_spec -v`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add .codex/skills/openusim-helper/scripts/openusim_helper/spec.py tests/openusim_helper/test_spec.py
git commit -m "feat: add openusim-helper spec rendering"
```

## Chunk 3: Case Generation Helpers

### Task 6: Implement the three writer modules as thin wrappers over existing primitives

**Files:**
- Create: `./.codex/skills/openusim-helper/scripts/openusim_helper/topology_writer.py`
- Create: `./.codex/skills/openusim-helper/scripts/openusim_helper/workload_writer.py`
- Create: `./.codex/skills/openusim-helper/scripts/openusim_helper/network_attribute_writer.py`
- Create: `tests/openusim_helper/test_writers.py`
- Reference: `tools/openusim/topology_builder.py`
- Reference: `tools/openusim/workload_builder.py`
- Reference: `tools/openusim/network_attribute_builder.py`

- [ ] **Step 1: Write focused tests for writer behavior**

Cover at least:
- topology writer produces the expected four files in a target `case/` directory
- workload writer writes `traffic.csv`
- network-attribute writer writes `network_attribute.txt`
- writer functions accept explicit arguments prepared by the agent, not a JSON artifact path
- topology writer rejects unsupported topology families
- workload writer rejects unsupported workload algorithms unless a reference `traffic.csv` is provided
- `trace/debug` overlay is applied through `network_attribute_writer`
- wrapper code can reuse an old primitive without exposing old CLI semantics

- [ ] **Step 2: Run the test to verify it fails**

Run: `python3 -m unittest tests.openusim_helper.test_writers -v`
Expected: FAIL because the writer modules do not exist yet.

- [ ] **Step 3: Implement the three writer modules**

Preferred shape:

```python
def write_topology(case_dir: Path, topology_plan: dict) -> dict: ...
def write_workload(case_dir: Path, workload_plan: dict) -> dict: ...
def write_network_attributes(case_dir: Path, attribute_plan: dict) -> dict: ...
```

Implementation guidance:
- reuse existing generation logic where it fits
- translate from explicit in-memory plans into file outputs
- keep each writer narrowly scoped
- do not recreate the old CLI entrypoint
- make `network_attribute_writer` the only place that applies the normalized `trace/debug` overlay

- [ ] **Step 4: Run the test to verify it passes**

Run: `python3 -m unittest tests.openusim_helper.test_writers -v`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add .codex/skills/openusim-helper/scripts/openusim_helper/topology_writer.py .codex/skills/openusim-helper/scripts/openusim_helper/workload_writer.py .codex/skills/openusim-helper/scripts/openusim_helper/network_attribute_writer.py tests/openusim_helper/test_writers.py
git commit -m "feat: add openusim-helper case writers"
```

### Task 7: Implement case checking and generated-case contract docs

**Files:**
- Create: `./.codex/skills/openusim-helper/scripts/openusim_helper/case_checker.py`
- Create: `docs/skill-validation/openusim-helper/generated-case-contract.md`
- Create: `tests/openusim_helper/test_case_checker.py`

- [ ] **Step 1: Write focused tests for hard checking**

Cover at least:
- required file presence
- required CSV or text fields needed by downstream tools
- success output when the generated case is complete
- one-pass repair path for a small non-semantic issue
- refusal to silently apply a semantic-changing fix
- checker only validates file/output contract, not predicted simulation quality

- [ ] **Step 2: Run the test to verify it fails**

Run: `python3 -m unittest tests.openusim_helper.test_case_checker -v`
Expected: FAIL because `case_checker.py` does not exist yet.

- [ ] **Step 3: Implement the checker and document the contract**

Implement helpers such as:

```python
def check_case_contract(case_dir: Path) -> dict: ...
def try_small_case_repair(case_dir: Path) -> dict: ...
```

Document the generated-case contract in `generated-case-contract.md`.

- [ ] **Step 4: Run the test to verify it passes**

Run: `python3 -m unittest tests.openusim_helper.test_case_checker -v`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add .codex/skills/openusim-helper/scripts/openusim_helper/case_checker.py docs/skill-validation/openusim-helper/generated-case-contract.md tests/openusim_helper/test_case_checker.py
git commit -m "feat: add openusim-helper case checker"
```

## Chunk 4: Minimal End-To-End Proof

### Task 8: Add an end-to-end generation proof using a tiny experiment

**Files:**
- Create: `tests/openusim_helper/test_end_to_end.py`
- Modify: `./.codex/skills/openusim-helper/scripts/openusim_helper/contracts.py`
- Modify: `./.codex/skills/openusim-helper/scripts/openusim_helper/workspace.py`
- Modify: `./.codex/skills/openusim-helper/scripts/openusim_helper/spec.py`
- Modify: `./.codex/skills/openusim-helper/scripts/openusim_helper/topology_writer.py`
- Modify: `./.codex/skills/openusim-helper/scripts/openusim_helper/workload_writer.py`
- Modify: `./.codex/skills/openusim-helper/scripts/openusim_helper/network_attribute_writer.py`
- Modify: `./.codex/skills/openusim-helper/scripts/openusim_helper/case_checker.py`

- [ ] **Step 1: Write the end-to-end test**

Build a tiny experiment in a temp workspace:
- create a stable goal summary
- create a workspace
- create normalized generation plans
- write a first spec
- generate all case files
- run the checker
- update the case-output part of the spec

- [ ] **Step 2: Run the end-to-end test to verify it fails**

Run: `python3 -m unittest tests.openusim_helper.test_end_to_end -v`
Expected: FAIL until the pieces integrate cleanly.

- [ ] **Step 3: Fix integration gaps only**

Do not expand scope. Only patch whatever is necessary to let the minimal experiment flow complete.

- [ ] **Step 4: Run the end-to-end and unit tests**

Run: `python3 -m unittest tests.openusim_helper.test_skill_docs tests.openusim_helper.test_contracts tests.openusim_helper.test_workspace tests.openusim_helper.test_spec tests.openusim_helper.test_writers tests.openusim_helper.test_case_checker tests.openusim_helper.test_end_to_end -v`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add .codex/skills/openusim-helper tests/openusim_helper docs/skill-validation/openusim-helper
git commit -m "feat: complete openusim-helper v1 case generation path"
```

### Task 9: Run a manual repo-local skill smoke check

**Files:**
- Modify: `docs/skill-validation/openusim-helper/README.md`

- [ ] **Step 1: Write down the smoke procedure**

Document a short manual protocol:
- start a fresh Codex session in this repo
- confirm `openusim-helper` is discoverable
- give a broad first-turn request
- verify the skill asks about the goal instead of generating immediately
- walk a minimal topology/workload/parameter confirmation path
- confirm the skill only generates after explicit approval

- [ ] **Step 2: Run the smoke procedure manually**

Expected:
- the skill is discoverable from `./.codex/skills/openusim-helper/`
- first turn stays light
- the generated workspace matches the expected layout

- [ ] **Step 3: Record anything surprising**

If the smoke reveals wording or spec-write issues, fix only the minimal doc or code path required and rerun the affected unit tests.

- [ ] **Step 4: Final verification**

Run: `python3 -m unittest tests.openusim_helper.test_skill_docs tests.openusim_helper.test_contracts tests.openusim_helper.test_workspace tests.openusim_helper.test_spec tests.openusim_helper.test_writers tests.openusim_helper.test_case_checker tests.openusim_helper.test_end_to_end -v`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add docs/skill-validation/openusim-helper/README.md .codex/skills/openusim-helper tests/openusim_helper
git commit -m "docs: add openusim-helper smoke validation"
```

## Execution Notes

- Keep old `skills/openusim-*`, `skills/openusim/`, and `tools/openusim/` untouched unless a tiny targeted reuse patch is clearly justified.
- Prefer adding small new modules over threading new behavior through the old OpenUSim orchestration code.
- For doc-only or metadata-only tasks, do not force TDD. Verify through targeted doc tests or direct smoke checks.
- For Python behavior, keep tests focused and local to `tests/openusim_helper/`.
- Do not add a new monolithic CLI just to connect the pieces.
- Keep the normalized contract layer explicit; do not let the writers invent defaults independently.

## Ready State

Plan complete when:

- `openusim-helper` is discoverable as a repo-local skill
- the skill docs enforce the light first-turn, stepwise clarification behavior
- the helper modules can create a workspace, write a current spec, generate case files, and hard-check them
- the minimal end-to-end test passes
- the manual smoke check matches the intended user experience
