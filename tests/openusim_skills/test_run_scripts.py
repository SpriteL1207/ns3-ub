import tempfile
import unittest
from pathlib import Path
from unittest import mock

from openusim_run_experiment import case_checker, network_attribute_writer


class OpenUSimRunScriptsTest(unittest.TestCase):
    def fake_parameter_catalog(self):
        return {
            "source_root": "scratch/ub-quick-example",
            "extractor_version": "test",
            "entry_count": 3,
            "entries": [
                {
                    "parameter_key": "ns3::UbPort::UbDataRate",
                    "kind": "AddAttribute",
                    "value_type": "ns3::DataRateValue",
                    "default_value": "400000000000bps",
                    "module": "UbPort",
                    "category": "general",
                    "safety_sensitivity": "medium",
                    "tuning_stage": "guided",
                },
                {
                    "parameter_key": "UB_TRACE_ENABLE",
                    "kind": "GlobalValue",
                    "value_type": "runtime-unavailable",
                    "default_value": "false",
                    "module": "GlobalValue",
                    "category": "trace",
                    "safety_sensitivity": "low",
                    "tuning_stage": "instrumentation",
                },
                {
                    "parameter_key": "UB_PYTHON_SCRIPT_PATH",
                    "kind": "GlobalValue",
                    "value_type": "runtime-unavailable",
                    "default_value": "unset",
                    "module": "GlobalValue",
                    "category": "trace",
                    "safety_sensitivity": "low",
                    "tuning_stage": "instrumentation",
                },
            ],
        }

    def test_writer_emits_full_snapshot_from_query_catalog(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            case_dir = Path(temp_dir) / "case"
            with mock.patch.object(
                network_attribute_writer,
                "load_or_build_parameter_catalog",
                return_value=(self.fake_parameter_catalog(), Path("/tmp/catalog.json")),
            ):
                result = network_attribute_writer.write_network_attributes(
                    case_dir,
                    explicit_overrides={"ns3::UbPort::UbDataRate": "800Gbps"},
                    observability_overrides={"UB_TRACE_ENABLE": "true"},
                )
            text = Path(result["path"]).read_text(encoding="utf-8")
            self.assertIn('default ns3::UbPort::UbDataRate "800Gbps"', text)
            self.assertIn('global UB_TRACE_ENABLE "true"', text)
            self.assertIn('global UB_PYTHON_SCRIPT_PATH "scratch/ns-3-ub-tools/trace_analysis/parse_trace.py"', text)
            self.assertEqual(result["resolution_mode"], "full-catalog-snapshot")
            self.assertGreaterEqual(result["resolved_entry_count"], 3)
            self.assertEqual(result["applied_overrides"]["ns3::UbPort::UbDataRate"], "800Gbps")

    def test_checker_only_reports_missing_major_files(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            case_dir = Path(temp_dir) / "case"
            case_dir.mkdir()
            (case_dir / "network_attribute.txt").write_text("", encoding="utf-8")
            report = case_checker.check_case_files(case_dir, transport_channel_mode="on_demand")
            self.assertEqual(report["status"], "missing_files")
            self.assertIn("node.csv", report["missing_files"])
            self.assertNotIn("transport_channel.csv", report["missing_files"])
            self.assertEqual(report["next_action"], "stop")
