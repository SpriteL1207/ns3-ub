# Topology Options

Use this reference when the user needs a bounded topology choice for one planned OpenUSim experiment.

## Recommended choice format

- `1:` recommended topology A
- `2:` recommended topology B
- `3:` recommended topology C
- `4:` user free input

Only offer the three most relevant template choices.

## Supported planning families

### `ring`

- Good for simple contention and directional throughput questions.
- Usually the easiest starting point for a small first experiment.

### `full-mesh`

- Good when the user wants direct pair connectivity without intermediate switches.
- Useful for small-scale baseline comparisons.

### `nd-full-mesh`

- Good when the user already thinks in dimensions and wants structured direct connectivity.
- The planning surface expects dimension sizes, not an open-ended graph description.
- The closest repo-native example is `scratch/ns-3-ub-tools/user_topo_4x4_2DFM.py`.

### `clos-spine-leaf`

- Good for common leaf-spine fabrics and pod-level or host-to-host comparisons.
- The planning surface expects `leaf_count`, `spine_count`, and `hosts_per_leaf`.
- If the user says `2-layer Clos`, `leaf-spine`, or `spine-leaf`, prefer this family by default.
- The closest repo-native example is `scratch/ns-3-ub-tools/user_topo_2layer_clos.py`.

### `clos-fat-tree`

- Good when the user explicitly wants a canonical Clos family with a single `k` parameter.
- Prefer this only when the user explicitly asks for `fat-tree` or gives a `k`-style input.
- **No repo-native example script exists for this family.** At run stage, the agent must stop and ask the user whether to proceed via `clos-spine-leaf` parameterized to match the k-derived sizing, or provide a custom script. Do not silently fall back to another family's script.

### old case reference

- Use when the user already has a previous case and wants the new experiment to start from it conceptually.
- This is a bounded reference path, not an in-place edit mode.

## Mapping rule

- Do not invent a custom topology generator path when the user request can be mapped to an existing `net_sim_builder.py`-style topology.
- If the request cannot be bounded to a supported family, ask the user to restate it as a supported family or provide an old case reference.

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
