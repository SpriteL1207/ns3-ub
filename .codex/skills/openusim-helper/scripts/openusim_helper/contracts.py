from __future__ import annotations

from copy import deepcopy


SUPPORTED_TOPOLOGY_FAMILIES = (
    "ring",
    "full-mesh",
    "nd-full-mesh",
    "clos-spine-leaf",
    "clos-fat-tree",
)

SUPPORTED_WORKLOAD_ALGOS = (
    "ar_ring",
    "ar_nhr",
    "ar_rhd",
    "a2a_pairwise",
    "a2a_scatter",
)

SUPPORTED_PARAMETER_CATEGORIES = (
    "buffer/credit",
    "packet/cell",
    "routing/load-balance",
    "rate/timing",
)

TRACE_DEBUG_PRESETS = {
    "minimal": {
        "global UB_TRACE_ENABLE": "false",
        "global UB_TASK_TRACE_ENABLE": "false",
        "global UB_PACKET_TRACE_ENABLE": "false",
        "global UB_PORT_TRACE_ENABLE": "false",
        "global UB_RECORD_PKT_TRACE": "false",
        "global UB_PARSE_TRACE_ENABLE": "false",
    },
    "balanced": {
        "global UB_TRACE_ENABLE": "true",
        "global UB_TASK_TRACE_ENABLE": "true",
        "global UB_PACKET_TRACE_ENABLE": "false",
        "global UB_PORT_TRACE_ENABLE": "true",
        "global UB_RECORD_PKT_TRACE": "false",
        "global UB_PARSE_TRACE_ENABLE": "true",
    },
    "detailed": {
        "global UB_TRACE_ENABLE": "true",
        "global UB_TASK_TRACE_ENABLE": "true",
        "global UB_PACKET_TRACE_ENABLE": "true",
        "global UB_PORT_TRACE_ENABLE": "true",
        "global UB_RECORD_PKT_TRACE": "true",
        "global UB_PARSE_TRACE_ENABLE": "true",
    },
}


def _copy_mapping(value):
    if value is None:
        return {}
    if not isinstance(value, dict):
        raise ValueError("expected a mapping")
    return deepcopy(value)


def make_topology_plan(
    family: str | None,
    *,
    mode: str = "template",
    parameters: dict | None = None,
    routing: dict | None = None,
    transport_channel: dict | None = None,
    old_case_dir: str | None = None,
    user_confirmed: dict | None = None,
    agent_completed: dict | None = None,
):
    if mode == "template":
        if family not in SUPPORTED_TOPOLOGY_FAMILIES:
            raise ValueError(f"unsupported topology family: {family}")
    elif mode == "old_case_reference":
        if not old_case_dir:
            raise ValueError("old_case_reference requires old_case_dir")
        family = "old-case-reference"
    else:
        raise ValueError(f"unsupported topology mode: {mode}")

    return {
        "mode": mode,
        "family": family,
        "parameters": _copy_mapping(parameters),
        "routing": {"strategy": "shortest-path", **_copy_mapping(routing)},
        "transport_channel": {"mode": "precomputed", **_copy_mapping(transport_channel)},
        "old_case_dir": old_case_dir,
        "user_confirmed": _copy_mapping(user_confirmed),
        "agent_completed": _copy_mapping(agent_completed),
    }


def make_workload_plan(
    algorithm: str | None = None,
    *,
    mode: str = "template",
    parameters: dict | None = None,
    reference_csv: str | None = None,
    user_confirmed: dict | None = None,
    agent_completed: dict | None = None,
):
    if mode == "template":
        if algorithm not in SUPPORTED_WORKLOAD_ALGOS:
            raise ValueError(f"unsupported workload algorithm: {algorithm}")
    elif mode == "reference_csv":
        if not reference_csv:
            raise ValueError("reference_csv mode requires a CSV path")
        algorithm = "reference_csv"
    else:
        raise ValueError(f"unsupported workload mode: {mode}")

    return {
        "mode": mode,
        "algorithm": algorithm,
        "parameters": _copy_mapping(parameters),
        "reference_csv": reference_csv,
        "user_confirmed": _copy_mapping(user_confirmed),
        "agent_completed": _copy_mapping(agent_completed),
    }


def make_network_attribute_plan(
    *,
    resolution_mode: str = "full-catalog-snapshot",
    explicit_overrides: dict | None = None,
    derived_overrides: dict | None = None,
):
    if resolution_mode != "full-catalog-snapshot":
        raise ValueError(f"unsupported network attribute resolution mode: {resolution_mode}")
    return {
        "resolution_mode": resolution_mode,
        "explicit_overrides": _copy_mapping(explicit_overrides),
        "derived_overrides": _copy_mapping(derived_overrides),
    }


def make_trace_debug_plan(
    mode: str = "balanced",
    *,
    custom_overrides: dict | None = None,
    user_confirmed: dict | None = None,
    agent_completed: dict | None = None,
):
    if mode == "custom":
        attribute_overrides = _copy_mapping(custom_overrides)
    else:
        if mode not in TRACE_DEBUG_PRESETS:
            raise ValueError(f"unsupported trace/debug mode: {mode}")
        attribute_overrides = deepcopy(TRACE_DEBUG_PRESETS[mode])
        attribute_overrides.update(_copy_mapping(custom_overrides))

    return {
        "mode": mode,
        "target_file": "network_attribute.txt",
        "attribute_overrides": attribute_overrides,
        "user_confirmed": _copy_mapping(user_confirmed),
        "agent_completed": _copy_mapping(agent_completed),
    }


def build_generation_bundle(
    *,
    experiment: dict,
    topology_plan: dict,
    workload_plan: dict,
    network_attribute_plan: dict,
    trace_debug_plan: dict,
):
    return {
        "experiment": deepcopy(experiment),
        "topology_plan": deepcopy(topology_plan),
        "workload_plan": deepcopy(workload_plan),
        "network_attribute_plan": deepcopy(network_attribute_plan),
        "trace_debug_plan": deepcopy(trace_debug_plan),
    }


def _missing_topology_fields(plan):
    params = plan.get("parameters", {})
    if plan["mode"] == "old_case_reference":
        return [] if plan.get("old_case_dir") else ["topology.old_case_dir"]
    family = plan["family"]
    required_map = {
        "ring": ("host_count",),
        "full-mesh": ("host_count",),
        "nd-full-mesh": ("dimension_sizes",),
        "clos-spine-leaf": ("leaf_count", "spine_count", "hosts_per_leaf"),
        "clos-fat-tree": ("fat_tree_k",),
    }
    return [f"topology.{field}" for field in required_map[family] if not params.get(field)]


def _missing_workload_fields(plan):
    if plan["mode"] == "reference_csv":
        return [] if plan.get("reference_csv") else ["workload.reference_csv"]
    params = plan.get("parameters", {})
    required = ("host_count", "comm_domain_size", "data_size")
    return [f"workload.{field}" for field in required if not params.get(field)]


def generation_ready(bundle):
    reasons = []
    experiment = bundle.get("experiment", {})
    for field in ("experiment_id", "workspace_dir", "case_dir"):
        if not experiment.get(field):
            reasons.append(f"experiment.{field}")
    if not experiment.get("generation_approved"):
        reasons.append("generation_approved")

    topology_plan = bundle.get("topology_plan")
    workload_plan = bundle.get("workload_plan")
    network_attribute_plan = bundle.get("network_attribute_plan")
    trace_debug_plan = bundle.get("trace_debug_plan")

    if not topology_plan:
        reasons.append("topology_plan")
    else:
        reasons.extend(_missing_topology_fields(topology_plan))

    if not workload_plan:
        reasons.append("workload_plan")
    else:
        reasons.extend(_missing_workload_fields(workload_plan))

    if not network_attribute_plan:
        reasons.append("network_attribute_plan")
    else:
        if network_attribute_plan.get("resolution_mode") != "full-catalog-snapshot":
            reasons.append("network_attribute.resolution_mode")

    if not trace_debug_plan:
        reasons.append("trace_debug_plan")
    else:
        if trace_debug_plan.get("target_file") != "network_attribute.txt":
            reasons.append("trace_debug.target_file")

    return len(reasons) == 0, reasons
