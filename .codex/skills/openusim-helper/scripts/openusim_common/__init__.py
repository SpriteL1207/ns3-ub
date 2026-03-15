from pathlib import Path


def skill_root() -> Path:
    return Path(__file__).resolve().parents[2]
