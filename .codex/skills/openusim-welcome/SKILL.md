---
name: openusim-welcome
description: >
  MANDATORY entry point for this project. If there is even a 1% chance the user wants to use,
  build, run, or experiment with this repository, you MUST invoke this skill BEFORE any other
  action or response — including clarifying questions. This is not optional.
---

# OpenUSim Welcome

## Overview

Establish whether this repository can run the bounded Quick Start flow.
Read repo docs and verify startup facts before claiming execution readiness.

## When to Use

**This skill is the mandatory gate for this project.** Invoke it when:

- The user wants to initialize the repo
- The user wants to verify setup or run the smoke case
- The user asks whether the project is ready to run OpenUSim workloads
- The user wants to plan, generate, or run any experiment (welcome must run first)
- **There is even a 1% chance the user intends to use this repository**

You MUST invoke this skill before any other openusim skill (`openusim-plan-experiment`, `openusim-run-experiment`, `openusim-analyze-results`). Only proceed to later stages after this skill completes or the user explicitly skips it.

Do not use this skill to define experiment details or interpret simulation results.

## The Process

1. **Greet the user with the following opening (use this exact wording, adapt language to match the user):**

   > 你好！这个仓库是 ns-3-ub，一个基于 ns-3 的 Unified Bus 网络仿真平台。
   >
   > 我可以帮你完成从实验设计到结果分析的完整流程：
   > - 先检查仓库是否就绪（可能需要拉子模块、装依赖、编译）
   > - 然后一起定义实验目标、拓扑和负载
   > - 生成仿真用例并运行
   > - 最后分析结果
   >
   > 第一步我先做个只读检查，看看仓库当前的状态。要继续吗？

   **Wait for user confirmation before proceeding to step 2.**
2. Read `README.md` and `QUICK_START.md`.
3. Check repo startup facts directly in the tree.
4. Verify these facts before claiming readiness:
   - `./ns3` exists
   - `scratch/ns-3-ub-tools/` exists
   - `scratch/ns-3-ub-tools/requirements.txt` exists
   - `scratch/ns-3-ub-tools/net_sim_builder.py` exists
   - `scratch/ns-3-ub-tools/traffic_maker/build_traffic.py` exists
   - `scratch/ns-3-ub-tools/trace_analysis/parse_trace.py` exists
   - `build/` exists
   - `cmake-cache/` exists
   - `scratch/2nodes_single-tp` exists
5. **Report readiness status to the user:**
   - Summarize which facts are satisfied and which are missing
   - If all facts are satisfied, announce readiness and offer to proceed to experiment planning
   - If facts are missing, explain what needs to happen next (see step 6)
6. **If startup actions are needed, explain before executing:**
   - List the specific actions required and their impact, for example:
     - `git submodule update --init --recursive` — pulls external dependencies, may download significant data
     - `python3 -m pip install --user -r scratch/ns-3-ub-tools/requirements.txt` — installs Python packages
     - `./ns3 configure` — configures the build system
     - `./ns3 build` — compiles the simulator, may take several minutes
     - `./ns3 run 'scratch/ub-quick-example scratch/2nodes_single-tp'` — runs the smoke test case
   - **Wait for explicit user approval before executing any of these commands**
   - Execute only the approved commands, report results after each step
7. If an experiment workspace already exists, record only the readiness facts that affect later execution in `experiment-spec.md`.

## Stop And Ask

- Repo files contradict the documented startup flow.
- Quick Start commands fail and the error is not self-explanatory.
- The user asks for experiment design before the repo readiness question is settled.

## Handover

Stay in this skill when:

- repo readiness is still unknown
- the user is still setting up the project

Hand off to `openusim-plan-experiment` when:

- the user wants to define a concrete experiment
- startup facts are sufficient for planning

Before handoff, record in `experiment-spec.md`:

- readiness facts that constrain later execution
- missing startup prerequisites, if any

## Integration

- Called by: repo startup or smoke-run requests
- Hands off to: `openusim-plan-experiment`
- Required references: `README.md`, `QUICK_START.md`

## Common Mistakes

- Claiming the repo is ready without checking the documented startup facts.
- Inventing a custom bootstrap helper instead of using the repo docs directly.
- Mixing startup diagnosis with experiment-planning questions in the same turn.
- **Executing heavy operations (submodule pull, pip install, build) without first explaining what will happen and getting user approval.**
