# Topology Options

Use this reference when the user needs a bounded topology choice for `openusim-helper v1`.

## Recommended choice format

- `1:` recommended topology A
- `2:` recommended topology B
- `3:` recommended topology C
- `4:` user free input

Only offer the three most relevant template choices.

## Supported `v1` families

### `ring`

- Good for simple contention or directional throughput questions.
- Usually the easiest starting point for a small first generated case.

### `full-mesh`

- Good when the user wants direct pair connectivity without intermediate switches.
- Useful for small-scale baseline comparisons.

### `nd-full-mesh`

- Good when the user already thinks in dimensions and wants a structured full connectivity pattern.
- `v1` expects dimension sizes, not an open-ended graph description.

### `clos-spine-leaf`

- Good for common leaf/spine fabrics and pod-level or host-to-host comparisons.
- `v1` expects `leaf_count`, `spine_count`, and `hosts_per_leaf`.
- If the user says `2-layer Clos`, `leaf-spine`, or `spine-leaf`, prefer this family by default.

### `clos-fat-tree`

- Good when the user wants a canonical multi-path Clos family with a single `k` parameter.
- `v1` expects `fat_tree_k`.
- Prefer this only when the user explicitly asks for `fat-tree` or gives a `k`-style input.

### old case reference

- Use when the user already has a previous case and wants the new experiment to start from it conceptually.
- In `v1`, this is a bounded reference path, not an in-place edit mode.
