from __future__ import annotations

import importlib.util
import shutil
import sys
import tempfile
from functools import lru_cache
from pathlib import Path

from . import repo_root
from .contracts import SUPPORTED_WORKLOAD_ALGOS


@lru_cache(maxsize=1)
def load_traffic_backend():
    traffic_dir = repo_root() / "scratch" / "ns-3-ub-tools" / "traffic_maker"
    if not traffic_dir.is_dir():
        raise FileNotFoundError(f"missing traffic backend directory: {traffic_dir}")
    if str(traffic_dir) not in sys.path:
        sys.path.insert(0, str(traffic_dir))

    module_path = traffic_dir / "build_traffic.py"
    if not module_path.is_file():
        raise FileNotFoundError(f"missing traffic backend script: {module_path}")
    spec = importlib.util.spec_from_file_location("openusim_helper_build_traffic", module_path)
    module = importlib.util.module_from_spec(spec)
    assert spec.loader is not None
    spec.loader.exec_module(module)
    return module


def write_workload(case_dir: Path, workload_plan: dict) -> dict:
    case_dir = Path(case_dir)
    case_dir.mkdir(parents=True, exist_ok=True)
    mode = workload_plan["mode"]

    if mode == "reference_csv":
        reference_csv = Path(workload_plan["reference_csv"])
        if not reference_csv.is_file():
            raise FileNotFoundError(f"missing reference traffic CSV: {reference_csv}")
        target = case_dir / "traffic.csv"
        shutil.copy2(reference_csv, target)
        return {
            "mode": mode,
            "algorithm": "reference_csv",
            "traffic_csv": str(target),
            "generation_strategy": "reference_csv_copy",
        }

    algorithm = workload_plan["algorithm"]
    if algorithm not in SUPPORTED_WORKLOAD_ALGOS:
        raise ValueError(f"unsupported workload algorithm: {algorithm}")

    parameters = workload_plan.get("parameters", {})
    backend = load_traffic_backend()
    all_rank_table = backend.generate_all_rank_table(
        int(parameters["host_count"]),
        int(parameters["comm_domain_size"]),
        parameters.get("rank_mapping", "linear"),
    )
    logic_comm_pairs = backend.generate_logic_comm_pairs(
        algo=algorithm,
        comm_size=int(parameters["comm_domain_size"]),
        comm_bytes=backend.parse_size(parameters["data_size"]),
        scatter_k=int(parameters.get("scatter_k", 1)),
    )
    with tempfile.TemporaryDirectory() as temp_dir:
        backend.write_rdma_operations(
            temp_dir,
            all_rank_table,
            logic_comm_pairs,
            phase_delay_ns=int(parameters.get("phase_delay", 0)),
        )
        shutil.copy2(Path(temp_dir) / "traffic.csv", case_dir / "traffic.csv")
    return {
        "mode": mode,
        "algorithm": algorithm,
        "traffic_csv": str(case_dir / "traffic.csv"),
        "generation_strategy": f"template:{algorithm}",
    }
