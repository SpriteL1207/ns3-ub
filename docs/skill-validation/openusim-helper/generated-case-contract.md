# OpenUSim Helper generated case contract

This document records the `v1` generation output expected from `openusim-helper`.

## Workspace Shape

After explicit generation approval, the skill should create a new workspace:

```text
scratch/openusim-generated/<experiment-id>/
  experiment-spec.md
  case/
    network_attribute.txt
    node.csv
    topology.csv
    routing_table.csv
    transport_channel.csv
    traffic.csv
```

The generated workspace is for the current experiment only. `v1` should not reuse an older workspace for a new experiment.

## Single Spec Contract

`experiment-spec.md` is the current Markdown spec. It should contain these sections:

1. Goal
2. Topology
3. Workload
4. Network Parameters
5. Trace / Debug Options
6. Readiness / Status
7. Case Output

Sections `1` through `5` are the generation input truth. Sections `6` and `7` are derived status only.

## Supported Generation Surface

The spec must normalize into the bounded `v1` surface before generation:

- one supported topology family or one bounded old-case reference
- one supported workload mode or one bounded reference `traffic.csv`
- a small set of main network parameters
- one trace/debug mode expressed as attribute overrides

`v1` does not promise open-ended natural-language compilation beyond this surface.

## Network Attribute Contract

`network_attribute.txt` must be written as one full resolved snapshot for the current experiment case.

That means:

- parameter names and runtime defaults come from the project parameter catalog generated through `./ns3` queries
- user-confirmed key parameters override those defaults
- agent-derived values may fill in non-key parameters when needed for a concrete case
- trace/debug mode is applied as the last bounded overlay
- the file must not depend on inheriting lines from `scratch/2nodes_single-tp/network_attribute.txt`

## Trace / Debug Contract

Trace and debug settings are an overlay inside `network_attribute.txt`.

Default user-facing choices are:

- `最小模式`
- `平衡模式`
- `详细模式`
- `用户自行指定`

The default recommendation is `平衡模式`.

`v1` should not create a separate trace/debug output file.

## Final Approval Contract

The case must not be generated until the user explicitly approves the final summary.

That approval gate should happen after:

- topology is narrowed to the supported surface
- workload is narrowed to the supported surface
- relevant parameter categories are confirmed
- trace/debug choice is confirmed

If the user asks to generate before those conditions are met, `v1` should not begin generation-side actions. It should stay in clarification mode and ask only for the smallest remaining blocking decision.

## Checker Contract

The checker is a light hard gate on file format only.

It may check:

- required output files exist
- CSV/TXT files are non-empty where required
- expected columns or line structures are present
- cross-file references are mechanically compatible

It must not act as:

- a throughput judge
- a congestion predictor
- a topology-performance reviewer

Readiness for generation means the spec is complete enough and approved. Checker success means the generated files satisfy the format contract.
