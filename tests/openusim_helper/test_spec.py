import tempfile
import unittest
from pathlib import Path

from openusim_helper import spec


class OpenUSimHelperSpecTest(unittest.TestCase):
    def model(self):
        return {
            "title": "2026-03-15 Ring Throughput Study",
            "goal": {
                "summary": "Measure ring throughput under a balanced collective workload.",
                "user_confirmed": [
                    "Question: can a ring topology sustain the offered load?",
                    "Success signal: the generated case should be ready for later execution.",
                ],
                "agent_completed": [],
            },
            "topology": {
                "user_confirmed": ["Family: ring", "Transport channel: precomputed"],
                "agent_completed": ["Routing default: shortest-path"],
            },
            "workload": {
                "user_confirmed": ["Mode: ar_ring", "Data size: 1KB"],
                "agent_completed": ["Rank mapping: linear"],
            },
            "network_parameters": {
                "user_confirmed": ["Packet/cell focus: default cell size"],
                "agent_completed": ["Derived: keep baseline buffer defaults"],
            },
            "trace_debug": {
                "user_confirmed": ["Mode: balanced"],
                "agent_completed": ["Adjustment: keep packet trace disabled"],
            },
            "readiness": {
                "ready": True,
                "missing": [],
                "needs_confirmation": [],
                "next_step": "Case generation is allowed after explicit approval.",
            },
            "case_output": {
                "workspace_dir": "/tmp/example",
                "case_dir": "/tmp/example/case",
                "generated_files": ["network_attribute.txt", "traffic.csv"],
                "spec_case_alignment": "Current case matches the latest confirmed spec.",
            },
        }

    def test_render_experiment_spec_has_seven_sections(self):
        text = spec.render_experiment_spec(self.model())
        for marker in (
            "## 1. Goal",
            "## 2. Topology",
            "## 3. Workload",
            "## 4. Network Parameters",
            "## 5. Trace / Debug Options",
            "## 6. Readiness / Status",
            "## 7. Case Output",
            "用户已确认：",
            "Agent 为生成 case 暂定：",
        ):
            self.assertIn(marker, text)

    def test_write_experiment_spec_persists_current_truth_only(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            spec_path = Path(temp_dir) / "experiment-spec.md"
            spec.write_experiment_spec(spec_path, self.model())
            text = spec_path.read_text(encoding="utf-8")
            self.assertIn("2026-03-15 Ring Throughput Study", text)
            self.assertNotIn("Revision History", text)
            self.assertIn("Current case matches the latest confirmed spec.", text)

    def test_summarize_old_case_reference_is_bounded(self):
        summary = spec.summarize_old_case_reference(
            {
                "topology": "ring with 4 hosts",
                "workload": "pairwise all-to-all",
                "parameters": "balanced baseline",
                "missing_information": "final success signal",
            }
        )
        for marker in ("Topology:", "Workload:", "Parameters:", "Missing information:"):
            self.assertIn(marker, summary)


if __name__ == "__main__":
    unittest.main()
