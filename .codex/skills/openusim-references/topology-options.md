# Topology Options

Use this reference when the user needs a bounded topology choice for one planned OpenUSim experiment.

## Core Principle

**Always generate a new Python script** that calls `net_sim_builder.py` library. Do NOT copy or modify existing example scripts. Example scripts (`user_topo_*.py`) are code templates for reference only, not reusable tools.

## Recommended choice format

- `1:` recommended topology A
- `2:` recommended topology B
- `3:` recommended topology C
- `4:` user free input

Only offer the three most relevant template choices.

## Supported Topology Families

### `ring`

**When to use:**
- Simple contention and directional throughput questions
- Usually the easiest starting point for a small first experiment

**Planning parameters:**
- `host_num`: total number of hosts (integer)

**Code template reference:** See generation pattern below

### `full-mesh`

**When to use:**
- Direct pair connectivity without intermediate switches
- Small-scale baseline comparisons

**Planning parameters:**
- `host_num`: total number of hosts (integer)

**Code template reference:** See generation pattern below

### `nd-full-mesh`

**When to use:**
- User thinks in dimensions and wants structured direct connectivity
- 2D/3D mesh topologies

**Planning parameters:**
- `row_num`: number of rows (integer)
- `col_num`: number of columns (integer)
- Derived: `host_num = row_num * col_num`

**Code template reference:** `scratch/ns-3-ub-tools/user_topo_4x4_2DFM.py` (for pattern only, do not copy)

### `clos-spine-leaf`

**When to use:**
- Common leaf-spine fabrics
- Pod-level or host-to-host comparisons
- If user says `2-layer Clos`, `leaf-spine`, or `spine-leaf`, prefer this family by default

**Planning parameters:**
- `host_num`: total number of hosts (integer)
- `leaf_sw_num`: number of leaf switches (integer)
- Derived: `spine_sw_num = host_num // leaf_sw_num`
- Constraint: `host_num % leaf_sw_num == 0`

**Code template reference:** `scratch/ns-3-ub-tools/user_topo_2layer_clos.py` (for pattern only, do not copy)

### `clos-fat-tree`

**When to use:**
- User explicitly wants canonical Clos family with single `k` parameter
- Prefer this only when user explicitly asks for `fat-tree` or gives a `k`-style input

**Planning parameters:**
- `k`: fat-tree parameter (must be even integer)
- Derived: `pod_num = k`, `hosts_per_pod = (k/2)^2`, `host_num = k * (k/2)^2`
- Derived: `leaf_sw_num = k * (k/2)`, `spine_sw_num = (k/2)^2`

**Code template reference:** Use `clos-spine-leaf` generation pattern with derived parameters

### old case reference

**When to use:**
- User already has a previous case and wants the new experiment to start from it conceptually
- This is a bounded reference path, not an in-place edit mode

## Mapping rule

- Do not invent a custom topology generator path when the user request can be mapped to a supported family above
- If the request cannot be bounded to a supported family, ask the user to restate it as a supported family or provide an old case reference


## `net_sim_builder.py` Library Interface

`net_sim_builder.py` is a Python library (not a CLI). It provides the `NetworkSimulationGraph` class.

### Core API

```python
import net_sim_builder as netsim
import networkx as nx

# Create graph
graph = netsim.NetworkSimulationGraph()

# Add hosts (must be numbered 0, 1, 2, ... consecutively)
graph.add_netisim_host(node_id, forward_delay='1ns')

# Add switches (must be added after all hosts)
graph.add_netisim_node(node_id, forward_delay='1ns')

# Add links
graph.add_netisim_edge(u, v, bandwidth='400Gbps', delay='20ns', edge_count=1)

# Generate config files
graph.build_graph_config()
graph.gen_route_table(path_finding_algo=all_shortest_paths, multiple_workers=4)
graph.config_transport_channel(priority_list=[7, 8])
graph.write_config()
```

### Common parameters

- `bandwidth`: link bandwidth with unit, e.g. `400Gbps` (valid: bps, Kbps, Mbps, Gbps, Tbps)
- `delay`: link delay with unit, e.g. `20ns` (valid: ns, us, ms, s)
- `forward_delay`: switch forward delay, e.g. `1ns`
- `priority_list`: TP priority list, e.g. `[7, 8]`
- `path_finding_algo`: routing algorithm function, default `nx.all_shortest_paths`
- `multiple_workers`: parallel workers for routing, e.g. `4`

### Node numbering constraint

- Hosts must be numbered from 0 consecutively
- Hosts must be added before switches via `add_netisim_host()` then `add_netisim_node()`

### Path finding algorithm template

```python
def all_shortest_paths(G, source, target):
    try:
        return nx.all_shortest_paths(G, source, target)
    except nx.NetworkXNoPath:
        return []
```

## Generation Patterns

### Pattern: `clos-spine-leaf`

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
    host_num = 64  # substitute from spec
    leaf_sw_num = 8  # substitute from spec
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

    # Connect hosts to leaves
    host_per_leaf = host_num // leaf_sw_num
    for host_id in range(host_num):
        leaf_id = host_num + (host_id // host_per_leaf)
        graph.add_netisim_edge(host_id, leaf_id, bandwidth='400Gbps', delay='20ns')

    # Connect leaves to spines (full mesh)
    for leaf_idx in range(leaf_sw_num):
        for spine_idx in range(spine_sw_num):
            leaf_id = host_num + leaf_idx
            spine_id = host_num + leaf_sw_num + spine_idx
            graph.add_netisim_edge(leaf_id, spine_id, bandwidth='400Gbps', delay='20ns')

    # Generate config files
    graph.build_graph_config()
    graph.gen_route_table(path_finding_algo=all_shortest_paths, multiple_workers=4)
    graph.config_transport_channel(priority_list=[7, 8])
    graph.write_config()
```

### Pattern: `nd-full-mesh`

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
    row_num = 4  # substitute from spec
    col_num = 4  # substitute from spec
    host_num = row_num * col_num

    # Add hosts
    for host_id in range(host_num):
        graph.add_netisim_host(host_id, forward_delay='1ns')

    # Connect in 2D mesh
    for x in range(col_num):
        # Connect hosts in same row
        host_in_row = [x * row_num + y for y in range(row_num)]
        for i in range(row_num):
            for j in range(i + 1, row_num):
                graph.add_netisim_edge(host_in_row[i], host_in_row[j],
                                      bandwidth='400Gbps', delay='10ns')

        # Connect hosts in same column
        host_in_col = [y * col_num + x for y in range(row_num)]
        for i in range(col_num):
            for j in range(i + 1, col_num):
                graph.add_netisim_edge(host_in_col[i], host_in_col[j],
                                      bandwidth='400Gbps', delay='10ns')

    # Generate config files
    graph.build_graph_config()
    graph.gen_route_table(path_finding_algo=all_shortest_paths, multiple_workers=4)
    graph.config_transport_channel(priority_list=[7])
    graph.write_config()
```

### Pattern: `ring`

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
    host_num = 8  # substitute from spec

    # Add hosts
    for host_id in range(host_num):
        graph.add_netisim_host(host_id, forward_delay='1ns')

    # Connect in ring
    for host_id in range(host_num):
        next_host = (host_id + 1) % host_num
        graph.add_netisim_edge(host_id, next_host, bandwidth='400Gbps', delay='20ns')

    # Generate config files
    graph.build_graph_config()
    graph.gen_route_table(path_finding_algo=all_shortest_paths, multiple_workers=4)
    graph.config_transport_channel(priority_list=[7, 8])
    graph.write_config()
```

### Pattern: `full-mesh`

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
    host_num = 8  # substitute from spec

    # Add hosts
    for host_id in range(host_num):
        graph.add_netisim_host(host_id, forward_delay='1ns')

    # Connect all pairs (full mesh)
    for i in range(host_num):
        for j in range(i + 1, host_num):
            graph.add_netisim_edge(i, j, bandwidth='400Gbps', delay='20ns')

    # Generate config files
    graph.build_graph_config()
    graph.gen_route_table(path_finding_algo=all_shortest_paths, multiple_workers=4)
    graph.config_transport_channel(priority_list=[7, 8])
    graph.write_config()
```

### Pattern: `clos-fat-tree`

Use `clos-spine-leaf` pattern with derived parameters:

```python
# Derive parameters from k
k = 4  # substitute from spec (must be even)
pod_num = k
hosts_per_pod = (k // 2) ** 2
host_num = pod_num * hosts_per_pod
leaf_sw_num = pod_num * (k // 2)
spine_sw_num = (k // 2) ** 2

# Then use clos-spine-leaf pattern above with these derived values
```

