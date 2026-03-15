# Transport Channel Modes

Use this reference when discussing `transport_channel.mode`, especially `precomputed` and `on_demand`.

## Hard Facts

- In the topology generation path, `precomputed` writes `transport_channel.csv`.
- In the topology generation path, `on_demand` leaves `transport_channel.csv` unset.
- In the topology/config checker path, `transport_channel.csv` is required only for `precomputed`.
- Existing-case validation also requires `transport_channel.csv` only when the chosen mode is `precomputed`.

## Non-Implications

- Do not claim that `on_demand` is invalid just because `transport_channel.csv` is absent.
- Do not claim that every runnable case must contain `transport_channel.csv`.
- Do not describe `on_demand` as an incomplete artifact merely because the precomputed file is missing.

## Safe Wording

- `precomputed` means the generated case includes an explicit `transport_channel.csv`.
- `on_demand` means the case may omit `transport_channel.csv`, and that is expected for this mode.
- If we switch from `precomputed` to `on_demand`, the validation rule for `transport_channel.csv` changes with it.

## Unsafe Wording

- missing `transport_channel.csv` means the case is broken.
- `on_demand` still needs `transport_channel.csv` to be valid.
- `transport_channel.csv` is always mandatory.

## Authority

- `code/doc fact`: ` .codex/skills/openusim-helper/scripts/openusim_common/topology_primitives.py ` writes `transport_channel.csv` only in `precomputed`
- `code/doc fact`: ` .codex/skills/openusim-helper/scripts/openusim_helper/case_checker.py ` requires `transport_channel.csv` only when the mode is `precomputed`
