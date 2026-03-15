import unittest
from pathlib import Path


class OpenUSimHelperSkillDocsTest(unittest.TestCase):
    def repo_root(self):
        return Path(__file__).resolve().parents[2]

    def read_text(self, relative_path):
        return (self.repo_root() / relative_path).read_text(encoding="utf-8")

    def test_skill_bundle_exists_and_is_discoverable(self):
        repo_root = self.repo_root()
        self.assertTrue((repo_root / ".codex" / "skills" / "openusim-helper" / "SKILL.md").is_file())
        self.assertTrue((repo_root / ".codex" / "skills" / "openusim-helper" / "README.md").is_file())
        self.assertTrue(
            (repo_root / ".codex" / "skills" / "openusim-helper" / "agents" / "openai.yaml").is_file()
        )
        self.assertTrue((repo_root / "AGENTS.md").is_file())

    def test_skill_docs_define_openusim_helper_surface(self):
        skill_text = self.read_text(".codex/skills/openusim-helper/SKILL.md")
        for marker in (
            "openusim-helper",
            "first reply",
            "experiment-spec.md",
            "1/2/3/4",
            "4: user free input",
            "Internal control info stays internal.",
            "one smallest blocking question",
            "`semi-specified`",
            "`reference-based`",
            "If the user says `2-layer Clos`, `leaf-spine`, or `spine-leaf`, bind that directly to `clos-spine-leaf`.",
            "Do not ignore already-provided facts by falling back to a generic intake template.",
            "Keep the structure implicit rather than labeled.",
            "Avoid visible control labels such as `已知事实`, `当前步骤`, `下一步`, or `Decision` in normal replies.",
            "README.md",
            "QUICK_START.md",
            "not by inventing a separate bootstrap helper workflow.",
            "scratch/ns-3-ub-tools/requirements.txt",
            "./ns3 run 'scratch/ub-quick-example scratch/2nodes_single-tp'",
            "--PrintTypeIds",
            "--ClassName=...",
            "--PrintUbGlobals",
            "full resolved snapshot",
            "Do not rely on a hand-written static Unified Bus parameter table.",
            "read `references/domain/trace-observability.md` first",
            "read `references/domain/transport-channel-modes.md` first",
            "read `references/domain/throughput-evidence.md` first",
            "supported `v1` topology families",
            "supported `v1` workload modes",
            "explicit final user approval",
            "If the user says “generate now” before the gate is satisfied",
            "old case",
            "Do not expose internal labels such as `existing_case`, `prepared_case`, or `goal-to-experiment`.",
        ):
            self.assertIn(marker, skill_text)

    def test_repo_agents_prefers_openusim_helper_entrypoint(self):
        agents_text = self.read_text("AGENTS.md")
        for marker in (
            "openusim-helper",
            "do not route the user into legacy `openusim-*` skills",
            "The user-visible reply is not a policy summary.",
            "stay in clarification mode",
            "ask one question about what the user wants to learn from the simulation",
            "bind the user-provided facts first",
            "if the user says `2-layer Clos`, `leaf-spine`, or `spine-leaf`, bind that directly as `clos-spine-leaf`",
            "smallest blocking question",
            "Keep this structure implicit rather than labeled.",
            "`已知事实`",
            "Do not ignore already-provided facts by falling back to a generic intake template.",
            "README.md",
            "QUICK_START.md",
            "Do not depend on a separate bootstrap helper script",
            "git submodule update --init --recursive",
            "./ns3 build",
            "project parameter catalog",
            "full resolved snapshot",
            "hand-written static Unified Bus parameter table",
            "If the user asks to generate before this gate is satisfied",
            "PrintUbGlobals",
            "trace-observability.md",
            "transport-channel-modes.md",
            "throughput-evidence.md",
            "existing_case",
            "prepared_case",
            "goal-to-experiment",
        ):
            self.assertIn(marker, agents_text)

    def test_skill_references_exist(self):
        repo_root = self.repo_root()
        for relative_path in (
            ".codex/skills/openusim-helper/references/topology-options.md",
            ".codex/skills/openusim-helper/references/workload-options.md",
            ".codex/skills/openusim-helper/references/spec-rules.md",
            ".codex/skills/openusim-helper/references/domain/trace-observability.md",
            ".codex/skills/openusim-helper/references/domain/transport-channel-modes.md",
            ".codex/skills/openusim-helper/references/domain/throughput-evidence.md",
        ):
            self.assertTrue((repo_root / relative_path).is_file(), msg=f"missing {relative_path}")

        trace_card = self.read_text(".codex/skills/openusim-helper/references/domain/trace-observability.md")
        for marker in (
            "## Hard Facts",
            "## Non-Implications",
            "## Safe Wording",
            "## Unsafe Wording",
            "## Authority",
            "Do not claim that `detailed` trace itself lowers simulated throughput.",
        ):
            self.assertIn(marker, trace_card)

        transport_card = self.read_text(".codex/skills/openusim-helper/references/domain/transport-channel-modes.md")
        for marker in (
            "## Hard Facts",
            "## Non-Implications",
            "## Safe Wording",
            "## Unsafe Wording",
            "## Authority",
            "Do not claim that `on_demand` is invalid just because `transport_channel.csv` is absent.",
        ):
            self.assertIn(marker, transport_card)

        throughput_card = self.read_text(".codex/skills/openusim-helper/references/domain/throughput-evidence.md")
        for marker in (
            "## Hard Facts",
            "## Non-Implications",
            "## Safe Wording",
            "## Unsafe Wording",
            "## Authority",
            "Do not treat `throughput.csv` as an end-to-end task metric; it is per-port-direction evidence.",
        ):
            self.assertIn(marker, throughput_card)

    def test_validation_docs_capture_public_behavior(self):
        readme = self.read_text("docs/skill-validation/openusim-helper/README.md")
        public_behavior = self.read_text("docs/skill-validation/openusim-helper/public-behavior.md")
        contract_doc = self.read_text("docs/skill-validation/openusim-helper/generated-case-contract.md")

        for marker in (
            "broad first-turn request",
            "do not generate immediately",
            "goal is stable",
            "explicit user approval",
            "format-only",
            "not a performance judgment",
            "policy summary",
            "smallest blocking question",
            "semi-specified",
            "reference-based",
            "If the user says `2-layer Clos`, `leaf-spine`, or `spine-leaf`, bind that directly as `clos-spine-leaf`.",
            "Avoid control labels such as:",
            "`已知事实`",
            "QUICK_START.md",
            "not by requiring a separate bootstrap helper script",
            "scratch/ns-3-ub-tools/net_sim_builder.py",
            "./ns3 configure",
            "./ns3 run 'scratch/ub-quick-example scratch/2nodes_single-tp'",
            "project parameter catalog",
            "full resolved snapshot",
            "hand-written static Unified Bus parameter table",
            "If the user says `generate now` or equivalent before the gate is satisfied",
            "PrintUbGlobals",
            "trace-observability.md",
            "avoid claiming that `detailed` trace itself lowers simulated throughput",
            "transport-channel-modes.md",
            "throughput-evidence.md",
            "transport_channel.csv` is always mandatory",
            "distinguish per-port throughput from per-task throughput",
            "existing_case",
            "prepared_case",
        ):
            self.assertIn(marker, public_behavior)

        self.assertIn("openusim-helper", readme)
        self.assertIn("repo-local", readme)
        self.assertIn("not validated or packaged as an independent git submodule", readme)
        self.assertIn("generated case contract", contract_doc)
        self.assertIn("network_attribute.txt", contract_doc)
        self.assertIn("transport_channel.csv", contract_doc)


if __name__ == "__main__":
    unittest.main()
