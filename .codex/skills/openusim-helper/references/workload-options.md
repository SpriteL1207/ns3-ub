# Workload Options

Use this reference when the user needs a bounded workload choice for `openusim-helper v1`.

## Recommended choice format

- `1:` recommended workload A
- `2:` recommended workload B
- `3:` recommended workload C
- `4:` user free input

Only offer the three most relevant template choices.

## Supported `v1` modes

### `ar_ring`

- Ring AllReduce.
- Good for simple collective baselines and phase-ordered bandwidth questions.

### `ar_nhr`

- Neighbor-halving reduction AllReduce.
- Good when the user wants an AllReduce variant with a different phase structure.

### `ar_rhd`

- Recursive halving/doubling AllReduce.
- Good for users specifically comparing collective schedules.

### `a2a_pairwise`

- Pairwise all-to-all.
- Good for direct all-to-all traffic pressure and pattern comparisons.

### `a2a_scatter`

- Scatter-style all-to-all.
- Good when the user wants a bounded all-to-all variant with `scatter_k`.

### reference `traffic.csv`

- Use when the user already has a concrete workload file and wants `v1` to anchor on that instead of a built-in template.

## Mapping rule

If the user asks for `incast`, `hotspot`, or another custom pattern, do not invent a generator path.

Instead:

- map it clearly to one supported `v1` mode if that mapping is defensible
- or ask for a reference `traffic.csv`
