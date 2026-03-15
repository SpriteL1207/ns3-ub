import json
from pathlib import Path


ARTIFACT_FILENAMES = {
    "parameter_catalog": "parameter-catalog.json",
}


def project_dir(repo_root):
    return Path(repo_root) / ".openusim" / "project"


def artifact_filename(artifact_name):
    return ARTIFACT_FILENAMES[artifact_name]


def write_json(path, data):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        json.dumps(data, indent=2, sort_keys=True) + "\n",
        encoding="utf-8",
    )
    return path


def read_json(path):
    return json.loads(Path(path).read_text(encoding="utf-8"))


def write_project_artifact(repo_root, artifact_name, data):
    path = project_dir(repo_root) / artifact_filename(artifact_name)
    return write_json(path, data)
