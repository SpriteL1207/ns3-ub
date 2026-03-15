import tempfile
import unittest
from pathlib import Path
from unittest import mock

from openusim_helper import (
    case_checker,
    contracts,
    network_attribute_writer,
    spec,
    topology_writer,
    workload_writer,
    workspace,
)


class OpenUSimHelperEndToEndTest(unittest.TestCase):
    def fake_parameter_catalog(self):
        return {
            "source_root": "scratch/ub-quick-example",
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

    def test_minimal_generation_flow(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            experiment_id = workspace.make_experiment_slug(
                "2026-03-15",
                "ring throughput packet spray comparison study",
                set(),
            )
            workspace_dir = workspace.create_experiment_workspace(repo_root, experiment_id)
            case_dir = workspace.case_dir_for_workspace(workspace_dir)

            topology_plan = contracts.make_topology_plan(
                "ring",
                parameters={"host_count": 4},
                transport_channel={"mode": "precomputed"},
                user_confirmed={"family": "ring"},
            )
            workload_plan = contracts.make_workload_plan(
                "ar_ring",
                parameters={
                    "host_count": 4,
                    "comm_domain_size": 4,
                    "data_size": "1KB",
                },
                user_confirmed={"algorithm": "ar_ring"},
            )
            network_plan = contracts.make_network_attribute_plan(
                explicit_overrides={"ns3::UbPort::UbDataRate": "800Gbps"},
                derived_overrides={"UB_CC_ENABLED": "true"},
            )
            trace_plan = contracts.make_trace_debug_plan("balanced")
            bundle = contracts.build_generation_bundle(
                experiment={
                    "experiment_id": experiment_id,
                    "workspace_dir": str(workspace_dir),
                    "case_dir": str(case_dir),
                    "generation_approved": True,
                },
                topology_plan=topology_plan,
                workload_plan=workload_plan,
                network_attribute_plan=network_plan,
                trace_debug_plan=trace_plan,
            )

            ready, reasons = contracts.generation_ready(bundle)
            self.assertTrue(ready, msg=str(reasons))

            spec_model = {
                "title": experiment_id,
                "goal": {
                    "summary": "Measure ring throughput under a small collective workload.",
                    "user_confirmed": ["Question: can this ring setup be generated cleanly?"],
                    "agent_completed": [],
                },
                "topology": {
                    "user_confirmed": ["Family: ring"],
                    "agent_completed": ["Transport channel: precomputed"],
                },
                "workload": {
                    "user_confirmed": ["Algorithm: ar_ring", "Data size: 1KB"],
                    "agent_completed": [],
                },
                "network_parameters": {
                    "user_confirmed": ["Key override: ns3::UbPort::UbDataRate=800Gbps"],
                    "agent_completed": ["Derived: UB_CC_ENABLED=true"],
                },
                "trace_debug": {
                    "user_confirmed": ["Mode: balanced"],
                    "agent_completed": [],
                },
                "readiness": {
                    "ready": True,
                    "missing": [],
                    "needs_confirmation": [],
                    "next_step": "Generate the case.",
                },
                "case_output": {
                    "workspace_dir": str(workspace_dir),
                    "case_dir": str(case_dir),
                    "generated_files": [],
                    "spec_case_alignment": "No case generated yet.",
                },
            }
            spec.write_experiment_spec(workspace_dir / "experiment-spec.md", spec_model)

            topology_writer.write_topology(case_dir, topology_plan)
            workload_writer.write_workload(case_dir, workload_plan)
            with mock.patch.object(
                network_attribute_writer,
                "_load_or_build_parameter_catalog",
                return_value=(self.fake_parameter_catalog(), Path("/tmp/parameter-catalog.json")),
            ):
                network_attribute_writer.write_network_attributes(case_dir, network_plan, trace_plan)

            report = case_checker.check_case_contract(case_dir)
            self.assertEqual(report["status"], "ok")

            spec_model["case_output"] = {
                "workspace_dir": str(workspace_dir),
                "case_dir": str(case_dir),
                "generated_files": sorted(path.name for path in case_dir.iterdir() if path.is_file()),
                "spec_case_alignment": "Current case matches the latest confirmed spec.",
            }
            spec.write_experiment_spec(workspace_dir / "experiment-spec.md", spec_model)

            final_spec = (workspace_dir / "experiment-spec.md").read_text(encoding="utf-8")
            self.assertIn("Current case matches the latest confirmed spec.", final_spec)
            self.assertIn("network_attribute.txt", final_spec)


if __name__ == "__main__":
    unittest.main()
