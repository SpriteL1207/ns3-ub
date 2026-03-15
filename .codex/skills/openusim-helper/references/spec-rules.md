# Spec Rules

Use this reference when writing or updating `experiment-spec.md`.

## Core rule

- `experiment-spec.md` is the current experiment description.
- Sections `1` through `5` are generation inputs.
- Sections `6` and `7` are derived status only.

## Write-back timing

Write back only when:

- a major step gained stable new information
- the user confirmed that step
- generation output facts need to be recorded

Do not rewrite the whole spec on every turn.

## Source marking

Use natural language but keep two light labels visible:

- `用户已确认：`
- `Agent 为生成 case 暂定：`

## Old case rule

If an old case is used as reference:

- summarize it in the conversation first
- do not write it into the new spec until the user says what to keep and what to change

## Readiness rule

`ready for generation` means:

- topology is inside the supported `v1` surface
- workload is inside the supported `v1` surface
- main parameter choices are concrete enough
- trace/debug mode is chosen
- explicit final generation approval has been given
