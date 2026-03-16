# Trace Observability

Use this reference when discussing `trace/debug` choices, especially `minimal`, `balanced`, and `detailed`.

## Hard Facts

- `trace/debug` mode changes observability and logging overhead.
- `detailed` trace can increase wall-clock runtime, trace volume, and post-processing cost.
- `minimal` trace reduces observability compared with `balanced` or `detailed`.
- `trace/debug` mode is an attribute overlay on `network_attribute.txt`, not a separate simulation result.

## Non-Implications

- Do not claim that `detailed` trace itself lowers simulated throughput.
- Do not claim that `detailed` trace makes a topology less able to approach line rate.
- Do not treat stronger observability as a network-semantic performance regression.
- Do not treat weaker observability as evidence that performance is better.

## Safe Wording

- `detailed trace` is better for diagnosis, but it can make the run and post-processing slower.
- `balanced` is a good default when the user wants some observability without paying the highest logging cost.
- `minimal` is suitable when the user mainly wants a faster run and accepts weaker debugging evidence.

## Unsafe Wording

- `detailed trace` may itself reduce throughput.
- `detailed trace` is not suitable for judging whether the topology approaches line rate.
- turning on more trace means the simulated network performs worse.

## Authority

- `code/doc fact`: trace/debug is modeled as attribute overrides on `network_attribute.txt`
- `maintainer fact`: detailed trace affects runtime cost and observability, not simulated throughput semantics by itself
