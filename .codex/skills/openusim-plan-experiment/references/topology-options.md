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

### old case reference

- Use when the user already has a previous case and wants the new experiment to start from it conceptually.
- This is a bounded reference path, not an in-place edit mode.

## Mapping rule

- Do not invent a custom topology generator path when the user request can be mapped to an existing `net_sim_builder.py`-style topology.
- If the request cannot be bounded to a supported family, ask the user to restate it as a supported family or provide an old case reference.
