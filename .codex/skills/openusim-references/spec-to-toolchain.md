# Spec to Toolchain Mapping

Use this reference when translating a stable `experiment-spec.md` into concrete tool invocations during the run stage.

## Core Principle

**Always generate a new Python script** in the case directory that calls `net_sim_builder.py` library. Do NOT copy or modify existing example scripts (`user_topo_*.py`). These are code templates for reference only, not reusable tools.

## Topology Slot → Topology Generation

### Process

1. **Read topology family and parameters** from `experiment-spec.md`
2. **Select the matching code template** from `topology-options.md` Generation Patterns section
3. **Generate a new script** `{case_dir}/generate_topology.py`:
   - Copy the complete pattern from `topology-options.md`
   - Substitute spec parameters (host_num, leaf_sw_num, etc.)
   - Adjust bandwidth/delay if specified in Network Overrides
   - Adjust priority_list if specified in Network Overrides
4. **Run the script:** `python3 {case_dir}/generate_topology.py`
5. **Outputs:** `node.csv`, `topology.csv`, `routing_table.csv`, `transport_channel.csv`

### Important Rules

- **Do NOT** copy or modify `scratch/ns-3-ub-tools/user_topo_*.py` scripts
- **Do NOT** run `net_sim_builder.py` directly (it's a library, not a CLI)
- **Do NOT** reuse scripts across cases (each case gets its own generated script)
- **Always** use the complete pattern from `topology-options.md`, not abbreviated code

### Example

**From spec:**
```yaml
Topology:
  family: clos-spine-leaf
  host_num: 64
  leaf_sw_num: 8

Network Overrides:
  bandwidth: 200Gbps
  delay: 10ns
```

**Generate `{case_dir}/generate_topology.py`:**
```python
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).parent.parent.parent / "scratch/ns-3-ub-tools"))

import net_sim_builder as netsim
import networkx as nx

def all_shortest_paths(G, source, target):
    try:
        return nx.all_shortest_paths(G, source, target)
    except nx.NetworkXNoPath:
        return []

if __name__ == '__main__':
    graph = netsim.NetworkSimulationGraph()

    # Parameters from experiment-spec.md
    host_num = 64
    leaf_sw_num = 8
    spine_sw_num = host_num // leaf_sw_num

    # Add hosts (0 to host_num-1)
    for host_id in range(host_num):
        graph.add_netisim_host(host_id, forward_delay='1ns')

    # Add leaf switches
    for leaf_idx in range(leaf_sw_num):
        graph.add_netisim_node(host_num + leaf_idx, forward_delay='1ns')

    # Add spine switches
    for spine_idx in range(spine_sw_num):
        graph.add_netisim_node(host_num + leaf_sw_num + spine_idx, forward_delay='1ns')

    # Connect hosts to leaves (bandwidth/delay from Network Overrides)
    host_per_leaf = host_num // leaf_sw_num
    for host_id in range(host_num):
        leaf_id = host_num + (host_id // host_per_leaf)
        graph.add_netisim_edge(host_id, leaf_id, bandwidth='200Gbps', delay='10ns')

    # Connect leaves to spines (full mesh)
    for leaf_idx in range(leaf_sw_num):
        for spine_idx in range(spine_sw_num):
            leaf_id = host_num + leaf_idx
            spine_id = host_num + leaf_sw_num + spine_idx
            graph.add_netisim_edge(leaf_id, spine_id, bandwidth='200Gbps', delay='10ns')

    # Generate config files
    graph.build_graph_config()
    graph.gen_route_table(path_finding_algo=all_shortest_paths, multiple_workers=4)
    graph.config_transport_channel(priority_list=[7, 8])
    graph.write_config()
```

**Then run:**
```bash
cd {case_dir}
python3 generate_topology.py
```

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
