from collections import defaultdict, deque
from pathlib import Path


DEFAULT_BANDWIDTH = "400Gbps"
DEFAULT_DELAY = "10ns"
DEFAULT_FORWARD_DELAY = "1ns"
DEFAULT_PRIORITY_LIST = [7]


class FallbackGraph:
    def __init__(self):
        self.nodes = {}
        self.edges = []

    def add_netisim_host(self, node_id, forward_delay=DEFAULT_FORWARD_DELAY, **_attr):
        self.nodes[node_id] = {"type": "host", "forward_delay": forward_delay}

    def add_netisim_node(self, node_id, forward_delay=DEFAULT_FORWARD_DELAY, **_attr):
        self.nodes[node_id] = {"type": "node", "forward_delay": forward_delay}

    def add_netisim_edge(self, u, v, bandwidth=DEFAULT_BANDWIDTH, delay=DEFAULT_DELAY, edge_count=1, **_attr):
        self.edges.append(
            {
                "u": u,
                "v": v,
                "bandwidth": bandwidth,
                "delay": delay,
                "edge_count": int(edge_count),
            }
        )


def build_ring_graph(graph, host_count):
    for node_id in range(host_count):
        graph.add_netisim_host(node_id, forward_delay=DEFAULT_FORWARD_DELAY)
    for node_id in range(host_count):
        graph.add_netisim_edge(
            node_id,
            (node_id + 1) % host_count,
            bandwidth=DEFAULT_BANDWIDTH,
            delay=DEFAULT_DELAY,
        )


def build_full_mesh_graph(graph, host_count):
    for node_id in range(host_count):
        graph.add_netisim_host(node_id, forward_delay=DEFAULT_FORWARD_DELAY)
    for left in range(host_count):
        for right in range(left + 1, host_count):
            graph.add_netisim_edge(
                left,
                right,
                bandwidth=DEFAULT_BANDWIDTH,
                delay=DEFAULT_DELAY,
            )


def add_unique_edge(graph, seen_edges, left, right):
    if left == right:
        return
    edge = (min(left, right), max(left, right))
    if edge in seen_edges:
        return
    seen_edges.add(edge)
    graph.add_netisim_edge(
        left,
        right,
        bandwidth=DEFAULT_BANDWIDTH,
        delay=DEFAULT_DELAY,
    )


def build_nd_full_mesh_graph(graph, dimension_sizes):
    if len(dimension_sizes) < 2:
        raise ValueError("nd-full-mesh requires at least two dimension sizes")
    if any(size < 2 for size in dimension_sizes):
        raise ValueError("each dimension size must be at least 2")

    host_count = 1
    for size in dimension_sizes:
        host_count *= size

    for node_id in range(host_count):
        graph.add_netisim_host(node_id, forward_delay=DEFAULT_FORWARD_DELAY)

    strides = []
    stride = 1
    for size in reversed(dimension_sizes):
        strides.insert(0, stride)
        stride *= size

    def coordinate_to_id(coordinate):
        return sum(value * axis_stride for value, axis_stride in zip(coordinate, strides))

    seen_edges = set()
    coordinates = [[]]
    for size in dimension_sizes:
        next_coordinates = []
        for prefix in coordinates:
            for value in range(size):
                next_coordinates.append(prefix + [value])
        coordinates = next_coordinates

    for axis, _axis_size in enumerate(dimension_sizes):
        groups = defaultdict(list)
        for coordinate in coordinates:
            key = tuple(value for index, value in enumerate(coordinate) if index != axis)
            groups[key].append(coordinate_to_id(coordinate))
        for node_ids in groups.values():
            for left_index in range(len(node_ids)):
                for right_index in range(left_index + 1, len(node_ids)):
                    add_unique_edge(
                        graph,
                        seen_edges,
                        node_ids[left_index],
                        node_ids[right_index],
                    )


def build_clos_spine_leaf_graph(graph, leaf_count, spine_count, hosts_per_leaf):
    next_node_id = 0
    leaf_ids = []
    spine_ids = []

    for _ in range(leaf_count * hosts_per_leaf):
        graph.add_netisim_host(next_node_id, forward_delay=DEFAULT_FORWARD_DELAY)
        next_node_id += 1

    for _ in range(leaf_count):
        leaf_ids.append(next_node_id)
        graph.add_netisim_node(next_node_id, forward_delay=DEFAULT_FORWARD_DELAY)
        next_node_id += 1

    for _ in range(spine_count):
        spine_ids.append(next_node_id)
        graph.add_netisim_node(next_node_id, forward_delay=DEFAULT_FORWARD_DELAY)
        next_node_id += 1

    for leaf_index, leaf_id in enumerate(leaf_ids):
        host_start = leaf_index * hosts_per_leaf
        for host_id in range(host_start, host_start + hosts_per_leaf):
            graph.add_netisim_edge(
                host_id,
                leaf_id,
                bandwidth=DEFAULT_BANDWIDTH,
                delay=DEFAULT_DELAY,
            )
        for spine_id in spine_ids:
            graph.add_netisim_edge(
                leaf_id,
                spine_id,
                bandwidth=DEFAULT_BANDWIDTH,
                delay=DEFAULT_DELAY,
            )


def build_clos_fat_tree_graph(graph, fat_tree_k):
    if fat_tree_k is None or fat_tree_k < 4 or fat_tree_k % 2 != 0:
        raise ValueError("clos-fat-tree requires an even fat_tree_k value >= 4")

    pods = fat_tree_k
    edge_per_pod = fat_tree_k // 2
    agg_per_pod = fat_tree_k // 2
    hosts_per_edge = fat_tree_k // 2
    core_groups = fat_tree_k // 2
    cores_per_group = fat_tree_k // 2

    total_hosts = (fat_tree_k**3) // 4

    for host_id in range(total_hosts):
        graph.add_netisim_host(host_id, forward_delay=DEFAULT_FORWARD_DELAY)

    next_node_id = total_hosts
    pod_edge_ids = []
    for _pod in range(pods):
        edge_ids = []
        for _edge in range(edge_per_pod):
            edge_ids.append(next_node_id)
            graph.add_netisim_node(next_node_id, forward_delay=DEFAULT_FORWARD_DELAY)
            next_node_id += 1
        pod_edge_ids.append(edge_ids)

    pod_agg_ids = []
    for _pod in range(pods):
        agg_ids = []
        for _agg in range(agg_per_pod):
            agg_ids.append(next_node_id)
            graph.add_netisim_node(next_node_id, forward_delay=DEFAULT_FORWARD_DELAY)
            next_node_id += 1
        pod_agg_ids.append(agg_ids)

    core_ids = []
    for _group in range(core_groups):
        group_ids = []
        for _core in range(cores_per_group):
            group_ids.append(next_node_id)
            graph.add_netisim_node(next_node_id, forward_delay=DEFAULT_FORWARD_DELAY)
            next_node_id += 1
        core_ids.append(group_ids)

    for pod_index in range(pods):
        for edge_index, edge_id in enumerate(pod_edge_ids[pod_index]):
            host_base = pod_index * edge_per_pod * hosts_per_edge + edge_index * hosts_per_edge
            for host_id in range(host_base, host_base + hosts_per_edge):
                graph.add_netisim_edge(
                    host_id,
                    edge_id,
                    bandwidth=DEFAULT_BANDWIDTH,
                    delay=DEFAULT_DELAY,
                )

    for pod_index in range(pods):
        for edge_id in pod_edge_ids[pod_index]:
            for agg_id in pod_agg_ids[pod_index]:
                graph.add_netisim_edge(
                    edge_id,
                    agg_id,
                    bandwidth=DEFAULT_BANDWIDTH,
                    delay=DEFAULT_DELAY,
                )

        for agg_index, agg_id in enumerate(pod_agg_ids[pod_index]):
            for core_id in core_ids[agg_index]:
                graph.add_netisim_edge(
                    agg_id,
                    core_id,
                    bandwidth=DEFAULT_BANDWIDTH,
                    delay=DEFAULT_DELAY,
                )


def expand_links(graph):
    links = []
    adjacency = defaultdict(list)
    port_counters = defaultdict(int)

    for edge in graph.edges:
        for _ in range(edge["edge_count"]):
            u = edge["u"]
            v = edge["v"]
            u_port = port_counters[u]
            v_port = port_counters[v]
            port_counters[u] += 1
            port_counters[v] += 1
            links.append(
                {
                    "u": u,
                    "u_port": u_port,
                    "v": v,
                    "v_port": v_port,
                    "bandwidth": edge["bandwidth"],
                    "delay": edge["delay"],
                }
            )
            adjacency[u].append({"neighbor": v, "local_port": u_port, "remote_port": v_port})
            adjacency[v].append({"neighbor": u, "local_port": v_port, "remote_port": u_port})

    return links, adjacency, port_counters


def shortest_distances(adjacency, start_node):
    distances = {start_node: 0}
    queue = deque([start_node])
    while queue:
        current = queue.popleft()
        for entry in adjacency[current]:
            neighbor = entry["neighbor"]
            if neighbor in distances:
                continue
            distances[neighbor] = distances[current] + 1
            queue.append(neighbor)
    return distances


def write_node_csv(output_dir, graph, port_counters):
    path = output_dir / "node.csv"
    with path.open("w", encoding="utf-8") as handle:
        handle.write("nodeId,nodeType,portNum,forwardDelay\n")
        for node_id in sorted(graph.nodes):
            node = graph.nodes[node_id]
            node_type = "DEVICE" if node["type"] == "host" else "SWITCH"
            handle.write(
                f"{node_id},{node_type},{port_counters.get(node_id, 0)},{node['forward_delay']}\n"
            )
    return path


def write_topology_csv(output_dir, links):
    path = output_dir / "topology.csv"
    with path.open("w", encoding="utf-8") as handle:
        handle.write("nodeId1,portId1,nodeId2,portId2,bandwidth,delay\n")
        for link in links:
            handle.write(
                f"{link['u']},{link['u_port']},{link['v']},{link['v_port']},{link['bandwidth']},{link['delay']}\n"
            )
    return path


def build_routing_rows(graph, adjacency):
    host_ids = sorted(node_id for node_id, node in graph.nodes.items() if node["type"] == "host")
    rows = []

    for src in sorted(graph.nodes):
        dist_from_src = shortest_distances(adjacency, src)
        for dst in host_ids:
            if src == dst or dst not in dist_from_src:
                continue

            shortest = dist_from_src[dst]
            dist_to_dst = shortest_distances(adjacency, dst)
            next_hops = {
                entry["neighbor"]
                for entry in adjacency[src]
                if dist_to_dst.get(entry["neighbor"]) == shortest - 1
            }
            out_ports = sorted(
                {
                    entry["local_port"]
                    for entry in adjacency[src]
                    if entry["neighbor"] in next_hops
                }
            )
            dst_predecessors = {
                entry["neighbor"]
                for entry in adjacency[dst]
                if dist_from_src.get(entry["neighbor"]) == shortest - 1
            }
            dst_ports = sorted(
                {
                    entry["local_port"]
                    for entry in adjacency[dst]
                    if entry["neighbor"] in dst_predecessors
                }
            )
            if not dst_ports:
                dst_ports = [0]

            metrics = " ".join(str(shortest) for _ in out_ports)
            out_ports_text = " ".join(str(value) for value in out_ports)
            for dst_port in dst_ports:
                rows.append(
                    {
                        "nodeId": src,
                        "dstNodeId": dst,
                        "dstPortId": dst_port,
                        "outPorts": out_ports_text,
                        "metrics": metrics,
                    }
                )

    return rows


def write_routing_csv(output_dir, routing_rows):
    path = output_dir / "routing_table.csv"
    with path.open("w", encoding="utf-8") as handle:
        handle.write("nodeId,dstNodeId,dstPortId,outPorts,metrics\n")
        for row in routing_rows:
            handle.write(
                f"{row['nodeId']},{row['dstNodeId']},{row['dstPortId']},{row['outPorts']},{row['metrics']}\n"
            )
    return path


def write_transport_csv(output_dir, routing_rows):
    path = output_dir / "transport_channel.csv"
    tpn_counters = defaultdict(int)

    with path.open("w", encoding="utf-8") as handle:
        handle.write("nodeId1,portId1,tpn1,nodeId2,portId2,tpn2,priority,metric\n")
        for row in routing_rows:
            node_id = row["nodeId"]
            dst_node_id = row["dstNodeId"]
            if node_id >= dst_node_id:
                continue
            out_ports = [int(value) for value in row["outPorts"].split() if value]
            metrics = [int(value) for value in row["metrics"].split() if value]
            for priority in DEFAULT_PRIORITY_LIST:
                for out_port, metric in zip(out_ports, metrics):
                    tpn1 = tpn_counters[node_id]
                    tpn2 = tpn_counters[dst_node_id]
                    handle.write(
                        f"{node_id},{out_port},{tpn1},{dst_node_id},{row['dstPortId']},{tpn2},{priority},{metric}\n"
                    )
                    tpn_counters[node_id] += 1
                    tpn_counters[dst_node_id] += 1
    return path


def write_generated_topology(graph, output_dir, transport_channel_mode):
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    links, adjacency, port_counters = expand_links(graph)
    routing_rows = build_routing_rows(graph, adjacency)

    node_csv = write_node_csv(output_dir, graph, port_counters)
    topology_csv = write_topology_csv(output_dir, links)
    routing_table_csv = write_routing_csv(output_dir, routing_rows)

    if transport_channel_mode == "precomputed":
        transport_channel_path = str(write_transport_csv(output_dir, routing_rows))
    else:
        transport_channel_path = None

    return {
        "node_csv": str(node_csv),
        "topology_csv": str(topology_csv),
        "routing_table_csv": str(routing_table_csv),
        "transport_channel_csv": transport_channel_path,
    }
