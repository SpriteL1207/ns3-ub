import unittest

from openusim_helper import contracts


class OpenUSimHelperContractsTest(unittest.TestCase):
    def test_supported_surfaces_are_explicit(self):
        self.assertEqual(
            contracts.SUPPORTED_TOPOLOGY_FAMILIES,
            (
                "ring",
                "full-mesh",
                "nd-full-mesh",
                "clos-spine-leaf",
                "clos-fat-tree",
            ),
        )
        self.assertEqual(
            contracts.SUPPORTED_WORKLOAD_ALGOS,
            (
                "ar_ring",
                "ar_nhr",
                "ar_rhd",
                "a2a_pairwise",
                "a2a_scatter",
            ),
        )

    def test_trace_debug_is_a_network_attribute_overlay(self):
        plan = contracts.make_trace_debug_plan("balanced")
        self.assertEqual(plan["target_file"], "network_attribute.txt")
        self.assertIn("global UB_TRACE_ENABLE", plan["attribute_overrides"])

    def test_network_attribute_plan_requires_full_snapshot_mode(self):
        plan = contracts.make_network_attribute_plan()
        self.assertEqual(plan["resolution_mode"], "full-catalog-snapshot")
        with self.assertRaises(ValueError):
            contracts.make_network_attribute_plan(resolution_mode="baseline-overlay")

    def test_generation_ready_rejects_missing_required_fields(self):
        bundle = contracts.build_generation_bundle(
            experiment={
                "experiment_id": "2026-03-15-ring",
                "workspace_dir": "/tmp/workspace",
                "case_dir": "/tmp/workspace/case",
                "generation_approved": False,
            },
            topology_plan=contracts.make_topology_plan(
                "ring",
                parameters={"host_count": 4},
            ),
            workload_plan=contracts.make_workload_plan(
                "ar_ring",
                parameters={"host_count": 4, "comm_domain_size": 4},
            ),
            network_attribute_plan=contracts.make_network_attribute_plan(),
            trace_debug_plan=contracts.make_trace_debug_plan(),
        )
        ready, reasons = contracts.generation_ready(bundle)
        self.assertFalse(ready)
        self.assertIn("workload.data_size", reasons)
        self.assertIn("generation_approved", reasons)

    def test_contracts_keep_user_and_agent_fields_separate(self):
        plan = contracts.make_topology_plan(
            "ring",
            parameters={"host_count": 4},
            user_confirmed={"family": "ring"},
            agent_completed={"transport_channel_mode": "precomputed"},
        )
        self.assertEqual(plan["user_confirmed"]["family"], "ring")
        self.assertEqual(plan["agent_completed"]["transport_channel_mode"], "precomputed")

    def test_unsupported_inputs_are_rejected(self):
        with self.assertRaises(ValueError):
            contracts.make_topology_plan("dragonfly")
        with self.assertRaises(ValueError):
            contracts.make_workload_plan("incast")


if __name__ == "__main__":
    unittest.main()
