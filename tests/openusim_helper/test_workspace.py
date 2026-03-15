import tempfile
import unittest
from pathlib import Path

from openusim_helper import workspace


class OpenUSimHelperWorkspaceTest(unittest.TestCase):
    def test_make_experiment_slug_uses_semantic_words(self):
        slug = workspace.make_experiment_slug(
            "2026-03-15",
            "ring throughput packet spray comparison study",
            set(),
        )
        self.assertEqual(slug, "2026-03-15-ring-throughput-packet-spray")

    def test_make_experiment_slug_tries_more_specific_name_before_v2(self):
        slug = workspace.make_experiment_slug(
            "2026-03-15",
            "ring throughput packet spray comparison study",
            {"2026-03-15-ring-throughput-packet-spray"},
        )
        self.assertEqual(slug, "2026-03-15-ring-throughput-packet-spray-comparison")

    def test_make_experiment_slug_falls_back_to_version_suffix(self):
        existing = {
            "2026-03-15-ring-throughput-packet-spray",
            "2026-03-15-ring-throughput-packet-spray-comparison",
            "2026-03-15-ring-throughput-packet-spray-comparison-study",
        }
        slug = workspace.make_experiment_slug(
            "2026-03-15",
            "ring throughput packet spray comparison study",
            existing,
        )
        self.assertEqual(slug, "2026-03-15-ring-throughput-packet-spray-v2")

    def test_create_experiment_workspace_creates_layout(self):
        with tempfile.TemporaryDirectory() as temp_dir:
            repo_root = Path(temp_dir)
            workspace_dir = workspace.create_experiment_workspace(repo_root, "2026-03-15-ring")
            self.assertEqual(workspace_dir, repo_root / "scratch" / "openusim-generated" / "2026-03-15-ring")
            self.assertTrue((workspace_dir / "experiment-spec.md").is_file())
            self.assertTrue((workspace_dir / "case").is_dir())


if __name__ == "__main__":
    unittest.main()
