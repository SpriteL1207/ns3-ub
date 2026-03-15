from __future__ import annotations

from pathlib import Path

from openusim_common import contracts as openusim_contracts
from openusim_common import parameter_catalog as openusim_parameter_catalog
from openusim_common import storage as openusim_storage
from openusim_common.network_attributes import infer_kind, render_line

from . import repo_root


PROJECT_TRACE_PARSER_PATH = Path("scratch/ns-3-ub-tools/trace_analysis/parse_trace.py")


def _normalize_override_key(parameter_key: str) -> str:
    if parameter_key.startswith("global "):
        return parameter_key[len("global ") :]
    if parameter_key.startswith("default "):
        return parameter_key[len("default ") :]
    return parameter_key


def _merge_normalized_overrides(*override_groups: dict) -> dict:
    merged = {}
    for group in override_groups:
        for parameter_key, value in group.items():
            merged[_normalize_override_key(parameter_key)] = value
    return merged


def _load_or_build_parameter_catalog(current_repo_root: Path) -> tuple[dict, Path]:
    catalog_path = openusim_storage.project_dir(current_repo_root) / openusim_storage.artifact_filename(
        "parameter_catalog"
    )
    if catalog_path.is_file():
        payload = openusim_storage.read_json(catalog_path)
        return openusim_contracts.validate_parameter_catalog(payload), catalog_path

    catalog = openusim_parameter_catalog.build_parameter_catalog(current_repo_root)
    written_path = openusim_storage.write_project_artifact(
        repo_root=current_repo_root,
        artifact_name="parameter_catalog",
        data=catalog,
    )
    return catalog, written_path


def _required_project_pins() -> dict:
    return {
        "UB_PYTHON_SCRIPT_PATH": PROJECT_TRACE_PARSER_PATH.as_posix(),
    }


def _resolve_values(catalog: dict, merged_overrides: dict) -> dict:
    resolved = {entry["parameter_key"]: entry["default_value"] for entry in catalog["entries"]}
    resolved.update(_required_project_pins())
    resolved.update(merged_overrides)
    return resolved


def _render_resolved_lines(catalog: dict, resolved_values: dict) -> list[str]:
    catalog_entry_by_key = {entry["parameter_key"]: entry for entry in catalog["entries"]}
    catalog_default_keys = sorted(
        key for key, entry in catalog_entry_by_key.items() if entry["kind"] == "AddAttribute"
    )
    catalog_global_keys = sorted(
        key for key, entry in catalog_entry_by_key.items() if entry["kind"] == "GlobalValue"
    )

    extra_default_keys = sorted(
        key for key in resolved_values if key not in catalog_entry_by_key and infer_kind(key) == "default"
    )
    extra_global_keys = sorted(
        key for key in resolved_values if key not in catalog_entry_by_key and infer_kind(key) == "global"
    )

    output_lines = [
        render_line("default", key, resolved_values[key])
        for key in catalog_default_keys + extra_default_keys
    ]
    if catalog_global_keys or extra_global_keys:
        output_lines.append("")
        output_lines.extend(
            render_line("global", key, resolved_values[key])
            for key in catalog_global_keys + extra_global_keys
        )
    return output_lines


def write_network_attributes(case_dir: Path, attribute_plan: dict, trace_debug_plan: dict | None = None) -> dict:
    case_dir = Path(case_dir)
    case_dir.mkdir(parents=True, exist_ok=True)

    current_repo_root = repo_root()
    catalog, catalog_path = _load_or_build_parameter_catalog(current_repo_root)
    merged_overrides = _merge_normalized_overrides(
        attribute_plan.get("derived_overrides", {}),
        attribute_plan.get("explicit_overrides", {}),
        trace_debug_plan.get("attribute_overrides", {}) if trace_debug_plan else {},
    )
    resolved_values = _resolve_values(catalog, merged_overrides)
    output_lines = _render_resolved_lines(catalog, resolved_values)
    output_path = case_dir / "network_attribute.txt"
    output_path.write_text("\n".join(output_lines) + "\n", encoding="utf-8")

    return {
        "path": str(output_path),
        "resolution_mode": attribute_plan.get("resolution_mode", "full-catalog-snapshot"),
        "parameter_catalog_source": str(catalog_path),
        "required_project_pins": _required_project_pins(),
        "resolved_entry_count": len(resolved_values),
        "applied_overrides": merged_overrides,
    }
