from __future__ import annotations

from pathlib import Path


SECTION_ORDER = (
    ("goal", "## 1. Goal"),
    ("topology", "## 2. Topology"),
    ("workload", "## 3. Workload"),
    ("network_parameters", "## 4. Network Parameters"),
    ("trace_debug", "## 5. Trace / Debug Options"),
    ("readiness", "## 6. Readiness / Status"),
    ("case_output", "## 7. Case Output"),
)


def _render_section_body(section):
    lines = []
    summary = section.get("summary")
    if summary:
        lines.append(summary)
        lines.append("")

    confirmed = section.get("user_confirmed", [])
    if confirmed:
        lines.append("用户已确认：")
        lines.extend(f"- {item}" for item in confirmed)
        lines.append("")

    completed = section.get("agent_completed", [])
    if completed:
        lines.append("Agent 为生成 case 暂定：")
        lines.extend(f"- {item}" for item in completed)
        lines.append("")

    if "ready" in section:
        lines.append(f"- Ready for generation: {'yes' if section['ready'] else 'no'}")
        for item in section.get("startup_checks", []):
            lines.append(f"- Startup: {item}")
        for item in section.get("missing", []):
            lines.append(f"- Missing: {item}")
        for item in section.get("needs_confirmation", []):
            lines.append(f"- Needs confirmation: {item}")
        next_step = section.get("next_step")
        if next_step:
            lines.append(f"- Next step: {next_step}")
        lines.append("")

    if "workspace_dir" in section:
        lines.append(f"- Workspace: {section['workspace_dir']}")
        lines.append(f"- Case directory: {section['case_dir']}")
        for name in section.get("generated_files", []):
            lines.append(f"- Generated: {name}")
        alignment = section.get("spec_case_alignment")
        if alignment:
            lines.append(f"- Alignment: {alignment}")
        lines.append("")

    return lines or ["- Not yet recorded.", ""]


def render_experiment_spec(model: dict) -> str:
    lines = [f"# {model.get('title', 'Experiment Spec')}", ""]
    for key, heading in SECTION_ORDER:
        lines.append(heading)
        lines.append("")
        lines.extend(_render_section_body(model.get(key, {})))
    return "\n".join(lines).rstrip() + "\n"


def write_experiment_spec(spec_path: Path, model: dict) -> None:
    Path(spec_path).write_text(render_experiment_spec(model), encoding="utf-8")


def summarize_old_case_reference(extracted: dict) -> str:
    return "\n".join(
        [
            f"Topology: {extracted.get('topology', 'unknown')}",
            f"Workload: {extracted.get('workload', 'unknown')}",
            f"Parameters: {extracted.get('parameters', 'unknown')}",
            f"Missing information: {extracted.get('missing_information', 'none')}",
        ]
    )
