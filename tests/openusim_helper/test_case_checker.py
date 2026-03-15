import tempfile
import unittest
from pathlib import Path
from unittest import mock

from openusim_helper import case_checker, contracts, network_attribute_writer, topology_writer, workload_writer


class OpenUSimHelperCaseCheckerTest(unittest.TestCase):
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

    def make_case(self, case_dir):
        topology_writer.write_topology(
            case_dir,
            contracts.make_topology_plan(
                "ring",
                parameters={"host_count": 4},
                transport_channel={"mode": "precomputed"},
            ),
        )
        workload_writer.write_workload(
            case_dir,
            contracts.make_workload_plan(
                "ar_ring",
                parameters={
                    "host_count": 4,
                    "comm_domain_size": 4,
                    "data_size": "1KB",
                },
            ),
        )
        with mock.patch.object(
            network_attribute_writer,
            "_load_or_build_parameter_catalog",
            return_value=(self.fake_parameter_catalog(), Path("/tmp/parameter-catalog.json")),
        ):
            network_attribute_writer.write_network_attributes(
                case_dir,
                contracts.make_network_attribute_plan(),
                contracts.make_trace_debug_plan("balanced"),
            )

    def test_checker_accepts_complete_case(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            case_dir = Path(temp_dir) / "case"
            self.make_case(case_dir)
            report = case_checker.check_case_contract(case_dir)
            self.assertEqual(report["status"], "ok")
            self.assertEqual(report["verified_errors"], [])

    def test_checker_can_repair_small_non_semantic_newline_issue(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            case_dir = Path(temp_dir) / "case"
            self.make_case(case_dir)
            traffic_path = case_dir / "traffic.csv"
            traffic_path.write_text(traffic_path.read_text(encoding="utf-8").rstrip("\n"), encoding="utf-8")

            report = case_checker.check_case_contract(case_dir)
            self.assertEqual(report["status"], "needs_repair")

            repair = case_checker.try_small_case_repair(case_dir)
            self.assertIn("restore trailing newline: traffic.csv", repair["applied_repairs"])

            repaired = case_checker.check_case_contract(case_dir)
            self.assertEqual(repaired["status"], "ok")

    def test_checker_refuses_semantic_changing_fix(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            case_dir = Path(temp_dir) / "case"
            self.make_case(case_dir)
            (case_dir / "transport_channel.csv").unlink()
            repair = case_checker.try_small_case_repair(case_dir)
            self.assertEqual(repair["applied_repairs"], [])
            self.assertIn("missing required file: transport_channel.csv", repair["blocked_repairs"])

    def test_checker_accepts_on_demand_case_without_transport_channel_csv(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            case_dir = Path(temp_dir) / "case"
            topology_writer.write_topology(
                case_dir,
                contracts.make_topology_plan(
                    "clos-spine-leaf",
                    parameters={"leaf_count": 2, "spine_count": 2, "hosts_per_leaf": 2},
                    transport_channel={"mode": "on_demand"},
                ),
            )
            workload_writer.write_workload(
                case_dir,
                contracts.make_workload_plan(
                    "ar_nhr",
                    parameters={
                        "host_count": 4,
                        "comm_domain_size": 4,
                        "data_size": "1KB",
                    },
                ),
            )
            with mock.patch.object(
                network_attribute_writer,
                "_load_or_build_parameter_catalog",
                return_value=(self.fake_parameter_catalog(), Path("/tmp/parameter-catalog.json")),
            ):
                network_attribute_writer.write_network_attributes(
                    case_dir,
                    contracts.make_network_attribute_plan(),
                    contracts.make_trace_debug_plan("balanced"),
                )

            self.assertFalse((case_dir / "transport_channel.csv").exists())

            report = case_checker.check_case_contract(case_dir, transport_channel_mode="on_demand")
            self.assertEqual(report["status"], "ok")
            self.assertEqual(report["verified_errors"], [])


if __name__ == "__main__":
    unittest.main()
