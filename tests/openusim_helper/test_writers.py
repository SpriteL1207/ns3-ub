import tempfile
import unittest
from pathlib import Path
from unittest import mock

from openusim_helper import contracts, network_attribute_writer, topology_writer, workload_writer
from openusim_common.network_attributes import validate_network_attribute_lines


class OpenUSimHelperWritersTest(unittest.TestCase):
    def repo_root(self):
        return Path(__file__).resolve().parents[2]

    def fake_parameter_catalog(self):
        return {
            "source_root": str(self.repo_root() / "scratch" / "ub-quick-example"),
            "extractor_version": "test",
            "entry_count": 4,
            "entries": [
                {
                    "parameter_key": "ns3::UbApp::EnableMultiPath",
                    "kind": "AddAttribute",
                    "value_type": "ns3::BooleanValue",
                    "default_value": "false",
                    "module": "UbApp",
                    "category": "general",
                    "safety_sensitivity": "medium",
                    "tuning_stage": "guided",
                },
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
                    "parameter_key": "UB_PYTHON_SCRIPT_PATH",
                    "kind": "GlobalValue",
                    "value_type": "runtime-unavailable",
                    "default_value": "/path/to/ns-3-ub-tools/trace_analysis/parse_trace.py",
                    "module": "GlobalValue",
                    "category": "trace",
                    "safety_sensitivity": "low",
                    "tuning_stage": "instrumentation",
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
            ],
        }

    def test_topology_writer_generates_template_files(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            case_dir = Path(temp_dir) / "case"
            plan = contracts.make_topology_plan(
                "ring",
                parameters={"host_count": 4},
                transport_channel={"mode": "precomputed"},
            )
            result = topology_writer.write_topology(case_dir, plan)
            for key in ("node_csv", "topology_csv", "routing_table_csv", "transport_channel_csv"):
                self.assertTrue(Path(result["paths"][key]).is_file(), msg=key)

    def test_topology_writer_rejects_unsupported_family(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            case_dir = Path(temp_dir) / "case"
            with self.assertRaises(ValueError):
                topology_writer.write_topology(
                    case_dir,
                    {"mode": "template", "family": "dragonfly", "parameters": {}, "routing": {}, "transport_channel": {}},
                )

    def test_workload_writer_generates_template_traffic(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            case_dir = Path(temp_dir) / "case"
            plan = contracts.make_workload_plan(
                "ar_ring",
                parameters={
                    "host_count": 4,
                    "comm_domain_size": 4,
                    "data_size": "1KB",
                    "phase_delay": 0,
                    "rank_mapping": "linear",
                },
            )
            result = workload_writer.write_workload(case_dir, plan)
            self.assertTrue(Path(result["traffic_csv"]).is_file())

    def test_workload_writer_rejects_unsupported_algorithm_without_reference(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            case_dir = Path(temp_dir) / "case"
            with self.assertRaises(ValueError):
                workload_writer.write_workload(
                    case_dir,
                    {
                        "mode": "template",
                        "algorithm": "incast",
                        "parameters": {
                            "host_count": 4,
                            "comm_domain_size": 4,
                            "data_size": "1KB",
                        },
                    },
                )

    def test_network_attribute_writer_writes_full_snapshot_with_pins_and_overrides(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            case_dir = Path(temp_dir) / "case"
            attribute_plan = contracts.make_network_attribute_plan(
                explicit_overrides={"ns3::UbPort::UbDataRate": "800Gbps"},
                derived_overrides={"ns3::UbPort::UbDataRate": "600Gbps"},
            )
            trace_plan = contracts.make_trace_debug_plan("balanced")
            with mock.patch.object(
                network_attribute_writer,
                "_load_or_build_parameter_catalog",
                return_value=(self.fake_parameter_catalog(), self.repo_root() / ".openusim" / "project" / "parameter-catalog.json"),
            ):
                result = network_attribute_writer.write_network_attributes(case_dir, attribute_plan, trace_plan)
            text = Path(result["path"]).read_text(encoding="utf-8")
            non_empty_lines = [line for line in text.splitlines() if line.strip()]
            self.assertEqual(result["resolved_entry_count"], len(non_empty_lines))
            self.assertGreater(len(non_empty_lines), 4)
            self.assertIn('default ns3::UbApp::EnableMultiPath "false"', text)
            self.assertIn('default ns3::UbPort::UbDataRate "800Gbps"', text)
            self.assertIn(
                'global UB_PYTHON_SCRIPT_PATH "scratch/ns-3-ub-tools/trace_analysis/parse_trace.py"',
                text,
            )
            self.assertIn('global UB_TRACE_ENABLE "true"', text)
            self.assertEqual(result["resolution_mode"], "full-catalog-snapshot")
            self.assertEqual(validate_network_attribute_lines(Path(result["path"])), [])

    def test_network_attribute_writer_appends_unknown_overrides(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            case_dir = Path(temp_dir) / "case"
            attribute_plan = contracts.make_network_attribute_plan(
                explicit_overrides={"UB_CC_ENABLED": "true"},
            )
            with mock.patch.object(
                network_attribute_writer,
                "_load_or_build_parameter_catalog",
                return_value=(self.fake_parameter_catalog(), self.repo_root() / ".openusim" / "project" / "parameter-catalog.json"),
            ):
                result = network_attribute_writer.write_network_attributes(case_dir, attribute_plan)

            text = Path(result["path"]).read_text(encoding="utf-8")
            self.assertIn('global UB_CC_ENABLED "true"', text)
            self.assertEqual(result["applied_overrides"], {"UB_CC_ENABLED": "true"})


if __name__ == "__main__":
    unittest.main()
