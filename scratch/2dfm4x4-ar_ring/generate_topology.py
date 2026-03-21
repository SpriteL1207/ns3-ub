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
    row_num = 4
    col_num = 4
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
