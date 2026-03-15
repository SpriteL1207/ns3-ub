# openusim-helper

`openusim-helper` is the repo-local OpenUSim companion skill for this `ns-3-ub` repository.

It is shipped with the main repository under ` .codex/skills/openusim-helper/ ` and is expected to evolve together with:

- `./ns3`
- `scratch/ub-quick-example`
- `scratch/ns-3-ub-tools/`
- Unified Bus runtime attribute/query support in the current tree

Its main job is to help an agent turn a natural-language simulation goal into:

- one current `experiment-spec.md`
- one generated case under `scratch/openusim-generated/<date>-<slug>/case`

It is not intended to be maintained as a separate git submodule or as a version-independent external package.

When the repository is not fully initialized yet, the helper should first follow the startup facts from `README.md` and `QUICK_START.md`, then continue with experiment clarification and case generation.
