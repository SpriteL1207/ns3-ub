from __future__ import annotations

import re
from pathlib import Path


STOP_WORDS = {
    "a",
    "an",
    "and",
    "for",
    "the",
    "with",
    "under",
    "study",
    "analysis",
    "simulation",
    "experiment",
}


def _semantic_tokens(goal_summary: str):
    tokens = [
        token.lower()
        for token in re.findall(r"[A-Za-z0-9]+", goal_summary)
        if token.lower() not in STOP_WORDS
    ]
    return tokens or ["experiment"]


def make_experiment_slug(date_text: str, goal_summary: str, existing_names: set[str] | None = None) -> str:
    existing_names = existing_names or set()
    tokens = _semantic_tokens(goal_summary)
    min_count = min(4, len(tokens))
    base_tokens = tokens[:min_count]
    candidate = f"{date_text}-{'-'.join(base_tokens)}"
    if candidate not in existing_names:
        return candidate

    for end in range(min_count + 1, len(tokens) + 1):
        candidate = f"{date_text}-{'-'.join(tokens[:end])}"
        if candidate not in existing_names:
            return candidate

    base = f"{date_text}-{'-'.join(base_tokens)}"
    version = 2
    while f"{base}-v{version}" in existing_names:
        version += 1
    return f"{base}-v{version}"


def create_experiment_workspace(repo_root: Path, experiment_id: str) -> Path:
    workspace_dir = Path(repo_root) / "scratch" / "openusim-generated" / experiment_id
    case_dir = workspace_dir / "case"
    case_dir.mkdir(parents=True, exist_ok=True)
    spec_path = workspace_dir / "experiment-spec.md"
    if not spec_path.exists():
        spec_path.write_text("# Experiment Spec\n\n", encoding="utf-8")
    return workspace_dir


def case_dir_for_workspace(workspace_dir: Path) -> Path:
    return Path(workspace_dir) / "case"
