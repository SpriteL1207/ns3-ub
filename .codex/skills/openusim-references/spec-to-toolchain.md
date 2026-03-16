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
