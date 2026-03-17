---
name: openusim-welcome
description: >
  MANDATORY entry point for this project. If there is even a 1% chance the user wants to use,
  build, run, or experiment with this repository, you MUST invoke this skill BEFORE any other
  action or response — including clarifying questions. This is not optional.
---

# OpenUSim Welcome

## Overview

A lightweight, fast gate that checks repo readiness in one pass. This skill should complete in a single turn when the repo is ready. Do not turn it into a multi-turn ceremony.

## When to Use

**This skill is the mandatory gate for this project.** Invoke it when:

- **There is even a 1% chance the user intends to use this repository**

You MUST invoke this skill before any other openusim skill (`openusim-plan-experiment`, `openusim-run-experiment`, `openusim-analyze-results`). Only proceed to later stages after this skill completes or the user explicitly skips it.

Do not use this skill to define experiment details or interpret simulation results.

## The Process

**Steps 1–3 happen in a single reply. Do not split them across multiple turns.**

1. **Greet and check in one pass.** Output the greeting below, then immediately check all startup facts silently (do not narrate each check). Append the result summary to the same reply.

   Greeting (use this exact wording, adapt language to match the user):

   > 你好！这个仓库是 ns-3-ub，一个基于 ns-3 的 Unified Bus 网络仿真平台。
   >
   > 我可以帮你完成从实验设计到结果分析的完整流程：先检查仓库就绪状态，然后定义实验、生成用例、运行仿真、分析结果。
   >
   > 我先快速检查一下仓库状态。

2. **Silently verify all startup facts** (do not list each check individually to the user):
   - `./ns3` exists
   - `scratch/ns-3-ub-tools/` exists
   - `scratch/ns-3-ub-tools/requirements.txt` exists
   - `scratch/ns-3-ub-tools/net_sim_builder.py` exists
   - `scratch/ns-3-ub-tools/traffic_maker/build_traffic.py` exists
   - `scratch/ns-3-ub-tools/trace_analysis/parse_trace.py` exists
   - `build/` exists
   - `cmake-cache/` exists
   - `scratch/2nodes_single-tp` exists

3. **Report result concisely:**
   - **If all facts pass:** "仓库就绪，可以开始实验设计。你想做什么样的仿真？"  — then hand off to `openusim-plan-experiment`.
   - **If some facts are missing:** list only the missing items, then explain what actions are needed and their impact. **Wait for user approval before executing any heavy operation** (submodule pull, pip install, configure, build).

   Available startup commands (only list the ones actually needed):
   - `git submodule update --init --recursive` — pulls external dependencies, may download significant data
   - `python3 -m pip install --user -r scratch/ns-3-ub-tools/requirements.txt` — installs Python packages
   - `./ns3 configure` — configures the build system
   - `./ns3 build` — compiles the simulator, may take several minutes
   - `./ns3 run 'scratch/ub-quick-example scratch/2nodes_single-tp'` — runs the smoke test case

**Do NOT:**
- Create `experiment-spec.md` during welcome — that is the plan stage's job
- Narrate each file check as a separate step
- Split the greeting and readiness check across multiple turns
- Read `README.md` or `QUICK_START.md` aloud to the user

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

Do not create `experiment-spec.md` during welcome. That is the plan stage's responsibility.

## Integration

- Called by: repo startup or smoke-run requests
- Hands off to: `openusim-plan-experiment`
- Required references: `README.md`, `QUICK_START.md`

## Common Mistakes

- Claiming the repo is ready without checking the documented startup facts.
- Inventing a custom bootstrap helper instead of using the repo docs directly.
- Mixing startup diagnosis with experiment-planning questions in the same turn.
- Executing heavy operations (submodule pull, pip install, build) without first explaining what will happen and getting user approval.
- **Turning welcome into a multi-turn ceremony** — greeting + check + result should be one reply when the repo is ready.
- **Creating `experiment-spec.md` during welcome** — that belongs to the plan stage.
- **Narrating each file check individually** — check silently, report the summary.
