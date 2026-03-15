from __future__ import annotations

from pathlib import Path


REQUIRED_FILES = {
    "network_attribute.txt": None,
    "node.csv": ("nodeId", "nodeType"),
    "topology.csv": ("nodeId1", "portId1"),
    "routing_table.csv": ("nodeId", "dstNodeId"),
    "transport_channel.csv": ("nodeId1", "portId1"),
    "traffic.csv": ("taskId", "dataSize(Byte)"),
}


def _required_files_for_mode(transport_channel_mode: str) -> dict:
    required_files = dict(REQUIRED_FILES)
    if transport_channel_mode == "on_demand":
        required_files.pop("transport_channel.csv", None)
    return required_files


def _read_header(path: Path):
    text = path.read_text(encoding="utf-8")
    first_line = text.splitlines()[0] if text.splitlines() else ""
    return text, first_line


def check_case_contract(case_dir: Path, transport_channel_mode: str = "precomputed") -> dict:
    case_dir = Path(case_dir)
    verified_errors = []
    repairable_gaps = []

    for name, header_markers in _required_files_for_mode(transport_channel_mode).items():
        path = case_dir / name
        if not path.is_file():
            verified_errors.append(f"missing required file: {name}")
            continue

        text, header = _read_header(path)
        if not text.endswith("\n"):
            repairable_gaps.append(f"restore trailing newline: {name}")

        if header_markers is None:
            continue

        if not all(marker in header for marker in header_markers):
            verified_errors.append(f"unexpected header in {name}: {header}")

    if verified_errors:
        status = "invalid"
    elif repairable_gaps:
        status = "needs_repair"
    else:
        status = "ok"

    return {
        "status": status,
        "verified_errors": verified_errors,
        "repairable_gaps": repairable_gaps,
        "suggested_repairs": repairable_gaps,
        "next_action": "repair" if repairable_gaps and not verified_errors else ("stop" if verified_errors else "ready"),
    }


def try_small_case_repair(case_dir: Path, transport_channel_mode: str = "precomputed") -> dict:
    case_dir = Path(case_dir)
    report = check_case_contract(case_dir, transport_channel_mode=transport_channel_mode)
    applied_repairs = []
    blocked_repairs = []

    if report["verified_errors"]:
        blocked_repairs.extend(report["verified_errors"])

    for repair in report["repairable_gaps"]:
        prefix = "restore trailing newline: "
        if not repair.startswith(prefix):
            blocked_repairs.append(repair)
            continue
        name = repair[len(prefix) :]
        path = case_dir / name
        text = path.read_text(encoding="utf-8")
        path.write_text(text + "\n", encoding="utf-8")
        applied_repairs.append(repair)

    return {
        "applied_repairs": applied_repairs,
        "blocked_repairs": blocked_repairs,
        "post_repair_report": check_case_contract(case_dir, transport_channel_mode=transport_channel_mode),
    }
