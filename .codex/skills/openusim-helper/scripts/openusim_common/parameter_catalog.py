import re
import subprocess
from pathlib import Path

from . import contracts


QUERY_PROGRAM = "scratch/ub-quick-example"
QUERY_CASE = "scratch/2nodes_single-tp"
QUERY_TIMEOUT_SECONDS = 30
EXTRACTOR_VERSION = "runtime-query-v1"

TYPE_ID_LINE_PATTERN = re.compile(r"^\s*(?P<type_id>ns3::(?:Ub\w+|TpConnectionManager))\s*$", re.MULTILINE)
ATTRIBUTE_BLOCK_PATTERN = re.compile(
    r"Attribute:\s*(?P<name>[^\n]+)\n"
    r"Description:\s*(?P<description>[^\n]*)\n"
    r"DataType:\s*(?P<value_type>[^\n]+)\n"
    r"Default:\s*(?P<default_value>[^\n]*)",
    re.MULTILINE,
)
GLOBAL_ENTRY_PATTERN = re.compile(
    r"Global:\s*(?P<name>UB_[^\n]+)\n"
    r"Description:\s*(?P<description>[^\n]*)\n"
    r"DataType:\s*(?P<value_type>[^\n]+)\n"
    r"Default:\s*(?P<default_value>[^\n]*)",
    re.MULTILINE,
)

CATEGORY_RULES = (
    (
        "trace",
        ("TRACE", "TRACE_ENABLE", "PARSE_TRACE", "RECORD_PKT_TRACE", "PYTHON_SCRIPT_PATH"),
    ),
    ("fault", ("FAULT",)),
    ("congestion-control", ("CAQM", "CC_", "CONGESTION")),
    ("routing", ("ROUT", "PATH", "SPRAY", "ALLOC")),
    ("flow-control", ("PFC", "CBFC", "FLOWCONTROL", "CREDIT")),
    ("buffer", ("BUFFER", "QUEUE", "OUTSTANDING")),
    ("transport", ("RETRANS", "RTO", "INFLIGHT", "TP", "JETTY")),
)

SAFETY_SENSITIVITY_BY_CATEGORY = {
    "trace": "low",
    "transport": "medium",
    "flow-control": "high",
    "buffer": "high",
    "routing": "high",
    "fault": "high",
    "congestion-control": "high",
    "general": "medium",
}

TUNING_STAGE_BY_CATEGORY = {
    "trace": "instrumentation",
    "transport": "guided",
    "flow-control": "advanced",
    "buffer": "advanced",
    "routing": "advanced",
    "fault": "advanced",
    "congestion-control": "advanced",
    "general": "guided",
}


def normalize_whitespace(text):
    return " ".join(text.split())


def classify_category(parameter_key, component_hint):
    signal = f"{parameter_key} {component_hint}".upper()
    for category, tokens in CATEGORY_RULES:
        if any(token in signal for token in tokens):
            return category
    return "general"


def build_entry(parameter_key, kind, value_type, default_value, module, component_hint):
    category = classify_category(parameter_key, component_hint)
    return {
        "parameter_key": parameter_key,
        "kind": kind,
        "value_type": value_type,
        "default_value": default_value,
        "module": module,
        "category": category,
        "safety_sensitivity": SAFETY_SENSITIVITY_BY_CATEGORY[category],
        "tuning_stage": TUNING_STAGE_BY_CATEGORY[category],
    }


def query_command(flag):
    return f"{QUERY_PROGRAM} {QUERY_CASE} {flag}"


def ensure_query_prerequisites(repo_root):
    repo_root = Path(repo_root).resolve()
    launcher_path = repo_root / "ns3"
    case_path = repo_root / QUERY_CASE
    if not launcher_path.is_file():
        raise FileNotFoundError(f"Missing ns-3 launcher: {launcher_path}")
    if not case_path.is_dir():
        raise FileNotFoundError(f"Missing query case directory: {case_path}")
    return launcher_path


def run_query(repo_root, flag):
    launcher_path = ensure_query_prerequisites(repo_root)
    command = [str(launcher_path), "run", query_command(flag)]
    try:
        result = subprocess.run(
            command,
            cwd=repo_root,
            capture_output=True,
            text=True,
            check=False,
            timeout=QUERY_TIMEOUT_SECONDS,
        )
    except subprocess.TimeoutExpired as exc:
        raise RuntimeError(f"Timed out while querying parameter catalog: {query_command(flag)}") from exc

    if result.returncode != 0:
        detail = result.stderr.strip() or result.stdout.strip() or "unknown error"
        raise RuntimeError(f"ns-3 query failed for {query_command(flag)}: {detail}")

    return result.stdout + result.stderr


def runtime_type_ids(repo_root):
    output = run_query(repo_root, "--PrintTypeIds")
    return sorted(set(TYPE_ID_LINE_PATTERN.findall(output)))


def runtime_attribute_entries(repo_root):
    entries = []

    for type_id in runtime_type_ids(repo_root):
        output = run_query(repo_root, f"--ClassName={type_id}")
        component_name = type_id.split("::")[-1]
        for match in ATTRIBUTE_BLOCK_PATTERN.finditer(output):
            parameter_key = f"{type_id}::{match.group('name')}"
            entries.append(
                build_entry(
                    parameter_key=parameter_key,
                    kind="AddAttribute",
                    value_type=normalize_whitespace(match.group("value_type")),
                    default_value=normalize_whitespace(match.group("default_value")),
                    module=component_name,
                    component_hint=component_name,
                )
            )

    return entries


def runtime_global_entries(repo_root):
    output = run_query(repo_root, "--PrintUbGlobals")
    entries = []

    for match in GLOBAL_ENTRY_PATTERN.finditer(output):
        parameter_key = match.group("name")
        entries.append(
            build_entry(
                parameter_key=parameter_key,
                kind="GlobalValue",
                value_type=normalize_whitespace(match.group("value_type")),
                default_value=normalize_whitespace(match.group("default_value")),
                module="GlobalValue",
                component_hint="GlobalValue",
            )
        )

    return entries


def build_parameter_catalog(repo_root):
    repo_root = Path(repo_root).resolve()
    entries = runtime_attribute_entries(repo_root)
    entries.extend(runtime_global_entries(repo_root))
    entries = sorted(entries, key=lambda entry: entry["parameter_key"])

    return contracts.validate_parameter_catalog(
        {
            "source_root": str(repo_root / QUERY_PROGRAM),
            "extractor_version": EXTRACTOR_VERSION,
            "entry_count": len(entries),
            "entries": entries,
        }
    )
