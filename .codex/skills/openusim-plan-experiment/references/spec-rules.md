# Spec Rules

Use this reference when writing or updating `experiment-spec.md`.

## Core rule

- `experiment-spec.md` is the only shared artifact across stage skills.
- It is the current experiment description, not a chat log.
- It must contain enough facts for the next stage skill to continue without hidden session memory.

## Write-back timing

Write back only when:

- a planning slot gained stable new information
- the user confirmed that slot
- a stage handoff needs durable facts

Do not rewrite the whole spec on every turn.

## Minimal template

Use a small stable structure so every stage skill can find the same facts quickly:

```md
# Experiment Spec

## Goal
- what the user wants to learn

## Topology
- chosen topology family
- concrete sizing facts
- old case reference, if any

## Workload
- chosen workload family or reference traffic file
- concrete scale facts

## Network Overrides
- resolved parameter overrides that matter for the run

## Observability
- chosen trace/debug posture

## Startup Readiness
- startup facts that constrain generation or execution

## Execution Record
- actual case path
- actual run command
- produced output artifacts
- unresolved explicit run errors

## Analysis Notes
- result findings that matter for the next iteration
- hypotheses to test next
```

Use empty sections only when the next stage clearly needs that slot.
Do not turn the spec into a transcript or a turn-by-turn checklist.

## Planning inputs

The planning surface must leave these durable facts before run handoff:

- experiment goal
- topology choice
- workload choice
- network parameter overrides
- observability choice
- explicit approval to generate or run

## Old case rule

If an old case is used as reference:

- summarize it in the conversation first
- do not write it into the new spec until the user says what to keep and what to change

## Readiness rule

`ready for run` means:

- topology is concrete enough to generate with repo-native tools
- workload is concrete enough to generate with repo-native tools
- main parameter choices are concrete enough for `network_attribute.txt`
- observability mode is chosen
- explicit run approval has been given
