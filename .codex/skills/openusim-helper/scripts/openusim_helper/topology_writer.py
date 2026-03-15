from __future__ import annotations

import shutil
from pathlib import Path

from openusim_common import topology_primitives

from .contracts import SUPPORTED_TOPOLOGY_FAMILIES


def _copy_case_file(source_dir: Path, case_dir: Path, name: str):
    source = source_dir / name
    target = case_dir / name
    if not source.is_file():
        raise FileNotFoundError(f"missing required source file: {source}")
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, target)
    return str(target)


def write_topology(case_dir: Path, topology_plan: dict) -> dict:
    case_dir = Path(case_dir)
    case_dir.mkdir(parents=True, exist_ok=True)
    mode = topology_plan["mode"]

    if mode == "old_case_reference":
        source_dir = Path(topology_plan["old_case_dir"])
        paths = {
            "node_csv": _copy_case_file(source_dir, case_dir, "node.csv"),
            "topology_csv": _copy_case_file(source_dir, case_dir, "topology.csv"),
            "routing_table_csv": _copy_case_file(source_dir, case_dir, "routing_table.csv"),
            "transport_channel_csv": _copy_case_file(source_dir, case_dir, "transport_channel.csv"),
        }
        return {"mode": mode, "family": topology_plan["family"], "paths": paths}

    family = topology_plan["family"]
    if family not in SUPPORTED_TOPOLOGY_FAMILIES:
        raise ValueError(f"unsupported topology family: {family}")

    graph = topology_primitives.FallbackGraph()
    parameters = topology_plan.get("parameters", {})
    transport_channel_mode = topology_plan.get("transport_channel", {}).get("mode", "precomputed")

    if family == "ring":
        topology_primitives.build_ring_graph(graph, int(parameters["host_count"]))
    elif family == "full-mesh":
        topology_primitives.build_full_mesh_graph(graph, int(parameters["host_count"]))
    elif family == "nd-full-mesh":
        topology_primitives.build_nd_full_mesh_graph(
            graph,
            [int(value) for value in parameters["dimension_sizes"]],
        )
    elif family == "clos-spine-leaf":
        topology_primitives.build_clos_spine_leaf_graph(
            graph,
            int(parameters["leaf_count"]),
            int(parameters["spine_count"]),
            int(parameters["hosts_per_leaf"]),
        )
    elif family == "clos-fat-tree":
        topology_primitives.build_clos_fat_tree_graph(graph, int(parameters["fat_tree_k"]))

    paths = topology_primitives.write_generated_topology(graph, case_dir, transport_channel_mode)
    return {"mode": mode, "family": family, "paths": paths}
