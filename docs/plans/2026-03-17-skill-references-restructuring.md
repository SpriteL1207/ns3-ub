# Skill References Restructuring Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Restructure OpenUSim skill references into a shared layer, enrich reference content with toolchain constraints and mappings, and fix Python code issues.

**Architecture:** Move all cross-stage reference files into `.codex/skills/openusim-references/`, update all SKILL.md and AGENTS.md paths, add a new `spec-to-toolchain.md` mapping document, enrich existing references with parameter constraints, and fix Python `repo_root()` / cache invalidation issues.

**Tech Stack:** Markdown (skill files), Python (scripts), git

---

### Task 1: Create shared references directory and move files

**Files:**
- Create: `.codex/skills/openusim-references/` (directory)
- Move: 6 files from per-skill `references/` dirs to shared dir
- Delete: empty `references/` dirs

**Step 1: Move plan-experiment references**

```bash
mkdir -p .codex/skills/openusim-references
git mv .codex/skills/openusim-plan-experiment/references/spec-rules.md .codex/skills/openusim-references/
git mv .codex/skills/openusim-plan-experiment/references/topology-options.md .codex/skills/openusim-references/
git mv .codex/skills/openusim-plan-experiment/references/workload-options.md .codex/skills/openusim-references/
```

**Step 2: Move analyze-results references**

```bash
git mv .codex/skills/openusim-analyze-results/references/throughput-evidence.md .codex/skills/openusim-references/
git mv .codex/skills/openusim-analyze-results/references/trace-observability.md .codex/skills/openusim-references/
git mv .codex/skills/openusim-analyze-results/references/transport-channel-modes.md .codex/skills/openusim-references/
```

**Step 3: Verify moves and commit**

```bash
ls .codex/skills/openusim-references/
# Expected: spec-rules.md  throughput-evidence.md  topology-options.md  trace-observability.md  transport-channel-modes.md  workload-options.md
ls .codex/skills/openusim-plan-experiment/references/ 2>/dev/null && echo "ERROR: dir not empty" || echo "OK: dir gone"
ls .codex/skills/openusim-analyze-results/references/ 2>/dev/null && echo "ERROR: dir not empty" || echo "OK: dir gone"
```

```bash
git add -A && git commit -m "refactor: move skill references to shared openusim-references directory"
```

---

### Task 2: Enrich topology-options.md with toolchain parameter constraints

**Files:**
- Modify: `.codex/skills/openusim-references/topology-options.md`

**Step 1: Append toolchain interface section**

Add after the existing `## Mapping rule` section:

```markdown
## Toolchain Interface

`net_sim_builder.py` is a Python library, not a CLI. It provides the `NetworkSimulationGraph` class.
To generate topology files, write a Python script that imports and calls this class (see example scripts below).

### Actual parameter names by family

#### `clos-spine-leaf`

- `host_num`: total number of hosts (integer)
- `leaf_sw_num`: number of leaf switches (integer)
- Derived: `spine_sw_num = host_num // leaf_sw_num`
- Constraint: `host_num % leaf_sw_num == 0`
- Example script: `scratch/ns-3-ub-tools/user_topo_2layer_clos.py`

#### `nd-full-mesh`

- `row_num`: number of rows (integer)
- `col_num`: number of columns (integer)
- Derived: `host_num = row_num * col_num`
- Example script: `scratch/ns-3-ub-tools/user_topo_4x4_2DFM.py`

### Common parameters

- `bandwidth`: link bandwidth with unit, e.g. `400Gbps` (valid: bps, Kbps, Mbps, Gbps, Tbps)
- `delay`: link delay with unit, e.g. `20ns` (valid: ns, us, ms, s)
- `forward_delay`: switch forward delay, e.g. `1ns`
- `priority_list`: TP priority list, e.g. `[7, 8]`
- `path_finding_algo`: default `nx.all_shortest_paths`
- `multiple_workers`: parallel workers for routing, e.g. `4`

### Node numbering constraint

- Hosts must be numbered from 0 consecutively
- Hosts must be added before switches via `add_netisim_host()` then `add_netisim_node()`

### Generation workflow

Example scripts (e.g. `user_topo_2layer_clos.py`) have hardcoded parameters.
When the user requests different sizing, the agent must generate a new parameterized script
based on the example, substituting the relevant parameters. The script produces:
`node.csv`, `topology.csv`, `routing_table.csv`, `transport_channel.csv`.
```

**Step 2: Verify and commit**

```bash
git diff .codex/skills/openusim-references/topology-options.md
git add .codex/skills/openusim-references/topology-options.md
git commit -m "docs: enrich topology-options with toolchain parameter constraints"
```

---

### Task 3: Enrich workload-options.md with parameter constraints

**Files:**
- Modify: `.codex/skills/openusim-references/workload-options.md`

**Step 1: Append parameter constraints section**

Add after the existing `## Mapping rule` section:

```markdown
## CLI Interface

Tool: `python3 scratch/ns-3-ub-tools/traffic_maker/build_traffic.py`

### Required arguments

| Argument | Type | Description |
|----------|------|-------------|
| `-n, --host-count` | int | Total number of hosts |
| `-c, --comm-domain-size` | int | Communication domain size (hosts per domain) |
| `-b, --data-size` | str | Per-participant data volume (B/KB/MB/GB) |
| `-a, --algo` | str | Collective algorithm name |

### Optional arguments

| Argument | Type | Default | Description |
|----------|------|---------|-------------|
| `--scatter-k` | int | 1 | For `a2a_scatter`: merge every k phases |
| `-d, --phase-delay` | int | 0 | Inter-phase delay in ns |
| `-o, --output-dir` | str | `./output` | Output root directory |
| `--rank-mapping` | str | `linear` | Rank assignment: `linear` or `round-robin` |

### Hard constraints

- `host_count % comm_domain_size == 0` (raises ValueError)
- `1 <= scatter_k < comm_domain_size`
- `data_size` must parse as number + unit (B/KB/MB/GB, case-insensitive)
- `rank_mapping` must be `linear` or `round-robin`

### Planning note

`rank_mapping` and `phase_delay` affect experiment results.
Collect them in the workload slot during planning, not as runtime afterthoughts.
```

**Step 2: Verify and commit**

```bash
git diff .codex/skills/openusim-references/workload-options.md
git add .codex/skills/openusim-references/workload-options.md
git commit -m "docs: enrich workload-options with CLI interface and parameter constraints"
```

---

### Task 4: Create spec-to-toolchain.md

**Files:**
- Create: `.codex/skills/openusim-references/spec-to-toolchain.md`

**Step 1: Write the mapping document**

```markdown
# Spec to Toolchain Mapping

Use this reference when translating a stable `experiment-spec.md` into concrete tool invocations during the run stage.

## Topology Slot → Topology Generation

1. Choose the example topology script closest to the requested family:
   - `clos-spine-leaf` → `scratch/ns-3-ub-tools/user_topo_2layer_clos.py`
   - `nd-full-mesh` → `scratch/ns-3-ub-tools/user_topo_4x4_2DFM.py`
   - `ring`, `full-mesh` → write a new script using `NetworkSimulationGraph` directly
2. Generate a new parameterized script in the case directory, substituting spec parameters.
3. Run the script: `python3 <generated_topo_script.py>`
4. Outputs: `node.csv`, `topology.csv`, `routing_table.csv`, `transport_channel.csv`

Do not run `net_sim_builder.py` directly — it is a library, not a CLI.

## Workload Slot → Traffic Generation

If the workload is a reference `traffic.csv`, copy it into the case directory.

Otherwise, map spec fields to CLI arguments:

```
python3 scratch/ns-3-ub-tools/traffic_maker/build_traffic.py \
  -n {host_num} \
  -c {comm_domain_size} \
  -b {data_size} \
  -a {algo} \
  --scatter-k {scatter_k} \
  -d {phase_delay} \
  --rank-mapping {rank_mapping} \
  -o {case_dir}
```

Copy the generated `traffic.csv` from the output subdirectory into the case directory.

## Network Overrides Slot → network_attribute.txt

Use the `write_network_attributes()` function from
`.codex/skills/openusim-run-experiment/scripts/openusim_run_experiment/network_attribute_writer.py`:

```python
from openusim_run_experiment.network_attribute_writer import write_network_attributes

write_network_attributes(
    case_dir=Path("<case_dir>"),
    explicit_overrides={"ns3::UbApp::SomeParam": "value", ...},
    observability_overrides={"ns3::UbApp::TraceParam": "value", ...},
)
```

This writes a full catalog snapshot. Explicit overrides take precedence over observability overrides.

## Observability Slot → Observability Overrides

Map the chosen trace/debug posture to concrete attribute overrides passed as
`observability_overrides` to `write_network_attributes()`.

Refer to `trace-observability.md` for the semantic constraints on trace modes.

## Execution

```bash
./ns3 run 'scratch/ub-quick-example {case_dir}'
```

Optional runtime switches (not part of experiment definition):
- `--mtp-threads=N`

## Spec Parameter Name Convention

Use toolchain-native parameter names in `experiment-spec.md`:
- `host_num` (not `host_count` or `num_hosts`)
- `leaf_sw_num` (not `leaf_count`)
- `comm_domain_size` (not `comm_size`)
- `data_size` with unit (e.g. `1GB`)

This avoids translation ambiguity at the plan→run boundary.
```

**Step 2: Verify and commit**

```bash
cat .codex/skills/openusim-references/spec-to-toolchain.md
git add .codex/skills/openusim-references/spec-to-toolchain.md
git commit -m "docs: add spec-to-toolchain mapping reference"
```

---

### Task 5: Update spec-rules.md with workload sub-fields

**Files:**
- Modify: `.codex/skills/openusim-references/spec-rules.md`

**Step 1: Update workload section in minimal template**

In the `## Minimal template` section, update the Workload block:

```markdown
## Workload
- chosen workload family or reference traffic file
- concrete scale facts
- rank_mapping (optional, default: linear)
- phase_delay (optional, default: 0)
```

**Step 2: Add parameter naming rule**

Add after `## Old case rule`:

```markdown
## Parameter naming rule

Use toolchain-native parameter names in the spec to avoid translation ambiguity:
- `host_num`, `leaf_sw_num`, `comm_domain_size`, `data_size`
- See `spec-to-toolchain.md` for the full mapping
```

**Step 3: Verify and commit**

```bash
git diff .codex/skills/openusim-references/spec-rules.md
git add .codex/skills/openusim-references/spec-rules.md
git commit -m "docs: add workload sub-fields and parameter naming rule to spec-rules"
```

---

### Task 6: Update SKILL.md reference paths

**Files:**
- Modify: `.codex/skills/openusim-plan-experiment/SKILL.md:69-83`
- Modify: `.codex/skills/openusim-run-experiment/SKILL.md:68-84`
- Modify: `.codex/skills/openusim-analyze-results/SKILL.md:53-66`

**Step 1: Update openusim-plan-experiment/SKILL.md**

Replace the Integration and References sections (lines 69-83):

```markdown
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
```

**Step 2: Update openusim-run-experiment/SKILL.md**

Replace the Integration and References sections (lines 68-84):

```markdown
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

## References

- `scratch/ns-3-ub-tools/net_sim_builder.py`
- `scratch/ns-3-ub-tools/user_topo_2layer_clos.py`
- `scratch/ns-3-ub-tools/user_topo_4x4_2DFM.py`
- `scratch/ns-3-ub-tools/traffic_maker/build_traffic.py`
- `../openusim-references/spec-to-toolchain.md`
- `../openusim-references/topology-options.md`
- `../openusim-references/workload-options.md`
- `../openusim-references/spec-rules.md`
```

**Step 3: Update openusim-analyze-results/SKILL.md**

Replace the Integration and References sections (lines 53-66):

```markdown
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
```

**Step 4: Verify and commit**

```bash
grep -n "references/" .codex/skills/openusim-plan-experiment/SKILL.md
grep -n "references/" .codex/skills/openusim-run-experiment/SKILL.md
grep -n "references/" .codex/skills/openusim-analyze-results/SKILL.md
# Expected: all paths now point to ../openusim-references/
```

```bash
git add .codex/skills/openusim-plan-experiment/SKILL.md .codex/skills/openusim-run-experiment/SKILL.md .codex/skills/openusim-analyze-results/SKILL.md
git commit -m "refactor: update SKILL.md reference paths to shared openusim-references"
```

---

### Task 7: Update AGENTS.md paths and add maintenance rule

**Files:**
- Modify: `AGENTS.md:153-161` (Domain Knowledge Routing)
- Modify: `AGENTS.md` (append new section)

**Step 1: Update Domain Knowledge Routing paths**

Replace lines 157-161:

```markdown
Current required card:

- for `trace/debug` semantics and recommendations: ` .codex/skills/openusim-references/trace-observability.md `
- for transport-channel semantics: ` .codex/skills/openusim-references/transport-channel-modes.md `
- for throughput evidence and line-rate interpretation: ` .codex/skills/openusim-references/throughput-evidence.md `
- for spec-to-toolchain mapping: ` .codex/skills/openusim-references/spec-to-toolchain.md `
```

**Step 2: Append Skill Reference Maintenance section**

Add at the end of AGENTS.md:

```markdown
## Skill Reference Maintenance

When tool scripts under `scratch/ns-3-ub-tools/` change their interface (parameters, CLI arguments, constraints, output schema), the corresponding reference files under `.codex/skills/openusim-references/` must be updated in the same commit.

Affected mappings:

- `net_sim_builder.py` API → `topology-options.md`, `spec-to-toolchain.md`
- `build_traffic.py` CLI → `workload-options.md`, `spec-to-toolchain.md`
- `trace_analysis/*.py` → `throughput-evidence.md`, `spec-to-toolchain.md`
```

**Step 3: Verify and commit**

```bash
grep "openusim-analyze-results/references" AGENTS.md
# Expected: no matches (old paths gone)
grep "openusim-references/" AGENTS.md
# Expected: 4+ matches (new paths)
```

```bash
git add AGENTS.md
git commit -m "refactor: update AGENTS.md paths to shared references, add maintenance rule"
```

---

### Task 8: Fix repo_root() hardcoded parents[5]

**Files:**
- Modify: `.codex/skills/openusim-run-experiment/scripts/openusim_run_experiment/__init__.py`

**Step 1: Replace hardcoded implementation**

Replace entire file content with:

```python
from pathlib import Path


def repo_root() -> Path:
    """Walk up from this file to find the repo root (directory containing the ns3 launcher)."""
    current = Path(__file__).resolve().parent
    while current != current.parent:
        if (current / "ns3").is_file():
            return current
        current = current.parent
    raise FileNotFoundError("Cannot find repo root (no 'ns3' launcher found)")
```

**Step 2: Verify**

```bash
cd /home/ytxing/workspace/github/ns-3-ub-skills
python3 -c "import sys; sys.path.insert(0, '.codex/skills/openusim-run-experiment/scripts'); from openusim_run_experiment import repo_root; print(repo_root())"
# Expected: /home/ytxing/workspace/github/ns-3-ub-skills
```

**Step 3: Commit**

```bash
git add .codex/skills/openusim-run-experiment/scripts/openusim_run_experiment/__init__.py
git commit -m "fix: replace hardcoded parents[5] with upward ns3 search in repo_root()"
```

---

### Task 9: Add cache invalidation to network_attribute_writer.py

**Files:**
- Modify: `.codex/skills/openusim-run-experiment/scripts/openusim_run_experiment/network_attribute_writer.py:128-131`

**Step 1: Add _cache_is_stale function**

Insert after `_write_json` function (after line 125):

```python

def _cache_is_stale(cache_path: Path) -> bool:
    """Return True if the cache is missing or older than the build/ directory."""
    if not cache_path.is_file():
        return True
    build_dir = repo_root() / "build"
    if not build_dir.is_dir():
        return True
    return build_dir.stat().st_mtime > cache_path.stat().st_mtime
```

**Step 2: Update load_or_build_parameter_catalog**

Replace line 130:
```python
    if cache_path.is_file():
```
with:
```python
    if not _cache_is_stale(cache_path):
```

**Step 3: Add override merge comment**

Replace line 174:
```python
    merged_overrides = _merge_overrides(observability_overrides, explicit_overrides)
```
with:
```python
    # Later groups override earlier ones: explicit user overrides take precedence
    merged_overrides = _merge_overrides(observability_overrides, explicit_overrides)
```

**Step 4: Verify syntax**

```bash
python3 -c "import sys; sys.path.insert(0, '.codex/skills/openusim-run-experiment/scripts'); import openusim_run_experiment.network_attribute_writer; print('OK')"
# Expected: OK
```

**Step 5: Commit**

```bash
git add .codex/skills/openusim-run-experiment/scripts/openusim_run_experiment/network_attribute_writer.py
git commit -m "fix: add cache invalidation and clarify override merge order"
```

---

### Task 10: Final verification

**Step 1: Verify no stale references remain**

```bash
grep -r "references/" .codex/skills/openusim-plan-experiment/ .codex/skills/openusim-analyze-results/ 2>/dev/null
# Expected: no output (no old paths)

grep -r "openusim-analyze-results/references" . --include="*.md" 2>/dev/null
# Expected: no output

grep -r "openusim-plan-experiment/references" . --include="*.md" 2>/dev/null
# Expected: no output
```

**Step 2: Verify shared references directory is complete**

```bash
ls -1 .codex/skills/openusim-references/
# Expected 7 files:
# spec-rules.md
# spec-to-toolchain.md
# throughput-evidence.md
# topology-options.md
# trace-observability.md
# transport-channel-modes.md
# workload-options.md
```

**Step 3: Verify Python imports still work**

```bash
python3 -c "
import sys; sys.path.insert(0, '.codex/skills/openusim-run-experiment/scripts')
from openusim_run_experiment import repo_root
from openusim_run_experiment.case_checker import check_case_files
from openusim_run_experiment.network_attribute_writer import load_or_build_parameter_catalog
print('repo_root:', repo_root())
print('All imports OK')
"
```

**Step 4: Verify old reference directories are gone**

```bash
test -d .codex/skills/openusim-plan-experiment/references && echo "ERROR: old dir exists" || echo "OK"
test -d .codex/skills/openusim-analyze-results/references && echo "ERROR: old dir exists" || echo "OK"
```
