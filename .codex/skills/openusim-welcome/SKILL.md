---
name: openusim-welcome
description: Use when the user wants to initialize this ns-3-ub repository, verify readiness, or run the bounded Quick Start smoke flow before planning an experiment.
---

# OpenUSim Welcome

## Overview

Establish whether this repository can run the bounded Quick Start flow.
Read repo docs and verify startup facts before claiming execution readiness.

## When to Use

- The user wants to initialize the repo.
- The user wants to verify setup or run the smoke case.
- The user asks whether the project is ready to run OpenUSim workloads.

Do not use this skill to define experiment details or interpret simulation results.

## The Process

1. Read `README.md` and `QUICK_START.md`.
2. Check repo startup facts directly in the tree.
3. Verify these facts before claiming readiness:
   - `./ns3` exists
   - `scratch/ns-3-ub-tools/` exists
   - `scratch/ns-3-ub-tools/requirements.txt` exists
   - `scratch/ns-3-ub-tools/net_sim_builder.py` exists
   - `scratch/ns-3-ub-tools/traffic_maker/build_traffic.py` exists
   - `scratch/ns-3-ub-tools/trace_analysis/parse_trace.py` exists
   - `build/` exists
   - `cmake-cache/` exists
   - `scratch/2nodes_single-tp` exists
4. If the user wants startup help, guide the bounded Quick Start commands:
   - `git submodule update --init --recursive`
   - `python3 -m pip install --user -r scratch/ns-3-ub-tools/requirements.txt`
   - `./ns3 configure`
   - `./ns3 build`
   - `./ns3 run 'scratch/ub-quick-example scratch/2nodes_single-tp'`
5. If an experiment workspace already exists, record only the readiness facts that affect later execution in `experiment-spec.md`.

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
