# Skill References Restructuring Design

Date: 2026-03-17

## Problem

The four OpenUSim stage skills store reference files in per-skill `references/` directories, but several references are needed across stages. Key issues:

1. Cross-stage references require absolute paths in AGENTS.md as glue
2. No shared location for cross-cutting documents like `spec-rules.md`
3. Plan-stage reference files lack tool-chain parameter constraints, causing plan→run misalignment
4. No explicit mapping from `experiment-spec.md` slots to actual tool invocations
5. Python code has a hardcoded `repo_root()` and no cache invalidation

## Decisions

- Scope: full improvement (directory restructure + content additions + Python fixes)
- Shared layer location: `.codex/skills/openusim-references/`
- Analyze knowledge cards: move to shared layer
- Approach: each SKILL.md keeps its own References section pointing to shared layer (no INDEX.md indirection)
- Coupling strategy: write concrete interfaces in references + explicit maintenance rule in AGENTS.md

## S1 — Directory Structure

Move 6 existing files, create 1 new file, delete 2 empty directories.

| From | To |
|------|----|
| `openusim-plan-experiment/references/spec-rules.md` | `openusim-references/spec-rules.md` |
| `openusim-plan-experiment/references/topology-options.md` | `openusim-references/topology-options.md` |
| `openusim-plan-experiment/references/workload-options.md` | `openusim-references/workload-options.md` |
| `openusim-analyze-results/references/throughput-evidence.md` | `openusim-references/throughput-evidence.md` |
| `openusim-analyze-results/references/trace-observability.md` | `openusim-references/trace-observability.md` |
| `openusim-analyze-results/references/transport-channel-modes.md` | `openusim-references/transport-channel-modes.md` |

New: `openusim-references/spec-to-toolchain.md`

Delete: `openusim-plan-experiment/references/`, `openusim-analyze-results/references/`

## S2 — Reference Content Changes

### topology-options.md

Append to existing content:

- Actual parameter names per family (aligned with `net_sim_builder.py` API):
  - `clos-spine-leaf`: `host_num`, `leaf_sw_num` (derived: `spine_sw_num = host_num // leaf_sw_num`)
  - `nd-full-mesh`: `row_num`, `col_num` (derived: `host_num = row_num * col_num`)
- Clarify that `net_sim_builder.py` is a library (not CLI); usage requires writing a topology script that imports `NetworkSimulationGraph`
- Clarify that example scripts (`user_topo_2layer_clos.py`, `user_topo_4x4_2DFM.py`) are hardcoded; agent must generate a new parameterized script based on the example
- Node numbering constraint: hosts numbered from 0 consecutively, hosts added before switches

### workload-options.md

Append to existing content:

- Hard constraint: `host_count % comm_domain_size == 0`
- `scatter_k` constraint: `1 <= scatter_k < comm_domain_size`
- `rank_mapping`: `linear` (default) or `round-robin`
- `phase_delay`: integer in ns, default 0
- `data_size` format: number + unit (B/KB/MB/GB)
- Note that `rank_mapping` and `phase_delay` affect results and should be collected in the workload slot during planning

### spec-to-toolchain.md (new)

Maps each `experiment-spec.md` slot to concrete tool invocations:

- Topology → write a topology script using `NetworkSimulationGraph`, run it to produce `node.csv`, `topology.csv`, `routing_table.csv`, `transport_channel.csv`
- Workload → `python3 build_traffic.py -n {host_num} -c {comm_domain_size} -b {data_size} -a {algo}` + optional `--scatter-k`, `-d`, `--rank-mapping`
- Network Overrides → `write_network_attributes(case_dir, explicit_overrides=...)` full catalog snapshot
- Observability → `write_network_attributes(case_dir, observability_overrides=...)` trace/debug overlay
- Execution → `./ns3 run 'scratch/ub-quick-example {case_dir}'`

### spec-rules.md

- Add `rank_mapping` and `phase_delay` as optional sub-fields of the workload slot
- Clarify that spec parameter names should match toolchain actual names (`host_num` not `host_count`)

## S3 — SKILL.md and AGENTS.md Path Updates

### SKILL.md References sections

**openusim-plan-experiment**: paths change to `../openusim-references/`, add `spec-to-toolchain.md`

**openusim-run-experiment**: add References section with `spec-to-toolchain.md`, `topology-options.md`, `workload-options.md`, `spec-rules.md`. Add note that topology generation requires writing a new script.

**openusim-analyze-results**: paths change to `../openusim-references/`

**openusim-welcome**: no reference changes

### AGENTS.md

- Domain Knowledge Routing paths → `.codex/skills/openusim-references/`
- New section: Skill Reference Maintenance rule — when tool scripts change interface, corresponding references must be updated in the same commit

## S4 — Python Fixes

### `__init__.py` — repo_root()

Replace `parents[5]` with upward search for `ns3` file:

```python
def repo_root() -> Path:
    current = Path(__file__).resolve().parent
    while current != current.parent:
        if (current / "ns3").is_file():
            return current
        current = current.parent
    raise FileNotFoundError("Cannot find repo root (no 'ns3' launcher found)")
```

### `network_attribute_writer.py` — cache invalidation

Add staleness check comparing cache mtime against `build/` directory mtime:

```python
def _cache_is_stale(cache_path: Path) -> bool:
    if not cache_path.is_file():
        return True
    build_dir = repo_root() / "build"
    if not build_dir.is_dir():
        return True
    return build_dir.stat().st_mtime > cache_path.stat().st_mtime
```

Replace `if cache_path.is_file()` with `if not _cache_is_stale(cache_path)` in `load_or_build_parameter_catalog()`.

### `network_attribute_writer.py` — override merge comment

Add clarifying comment above the merge call:

```python
# Later groups override earlier ones: explicit user overrides take precedence
merged_overrides = _merge_overrides(observability_overrides, explicit_overrides)
```
