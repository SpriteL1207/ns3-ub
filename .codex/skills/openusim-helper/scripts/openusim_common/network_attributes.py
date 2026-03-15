import re


LINE_PATTERN = re.compile(r'^(?P<kind>default|global)\s+(?P<key>\S+)\s+"(?P<value>.*)"\s*$')


def render_line(kind, key, value):
    return f'{kind} {key} "{value}"'


def infer_kind(parameter_key):
    if parameter_key.startswith("ns3::"):
        return "default"
    return "global"


def validate_network_attribute_lines(network_attribute_path):
    errors = []
    for line_number, raw_line in enumerate(
        network_attribute_path.read_text(encoding="utf-8").splitlines(),
        start=1,
    ):
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue
        if LINE_PATTERN.match(line) is None:
            errors.append(
                f"Invalid network_attribute.txt line {line_number}: {raw_line}"
            )
    return errors
