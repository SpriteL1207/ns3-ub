PARAMETER_CATALOG_REQUIRED_FIELDS = (
    "source_root",
    "extractor_version",
    "entry_count",
    "entries",
)

PARAMETER_CATALOG_ENTRY_FIELDS = (
    "parameter_key",
    "kind",
    "value_type",
    "default_value",
    "module",
    "category",
    "safety_sensitivity",
    "tuning_stage",
)


def validate_parameter_catalog(payload):
    missing_fields = [
        field for field in PARAMETER_CATALOG_REQUIRED_FIELDS if field not in payload
    ]
    if missing_fields:
        missing_list = ", ".join(missing_fields)
        raise ValueError(f"parameter_catalog is missing required fields: {missing_list}")

    for index, entry in enumerate(payload["entries"]):
        missing_fields = [
            field for field in PARAMETER_CATALOG_ENTRY_FIELDS if field not in entry
        ]
        if missing_fields:
            missing_list = ", ".join(missing_fields)
            raise ValueError(
                f"parameter_catalog entry {index} is missing required fields: {missing_list}"
            )
    return payload
