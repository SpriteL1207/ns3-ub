# Throughput Evidence

Use this reference when discussing throughput results, line-rate questions, or how to interpret `throughput.csv` and `task_statistics.csv`.

## Hard Facts

- `throughput.csv` is produced from `PortTrace_node_*_port_*.tr`.
- `throughput.csv` records per-port-direction throughput with `type` = `Rx` or `Tx`.
- `task_statistics.csv` records per-task throughput using `traffic.csv` data size plus task start/end times.
- The current helper does not automatically normalize measured throughput against configured line rate.

## Non-Implications

- Do not treat `throughput.csv` as an end-to-end task metric; it is per-port-direction evidence.
- Do not treat `taskThroughput(Gbps)` as the same thing as per-port `Rx/Tx throughput(Gbps)`.
- Do not claim “close to line rate” unless the comparison target and metric source are both stated.
- Do not assume the helper has already converted measured throughput into a line-rate ratio.

## Safe Wording

- `throughput.csv` tells us per-port Rx/Tx throughput; we still need to compare it against the configured port rate if the goal is line rate.
- `task_statistics.csv` is better for per-task completion and task-level throughput, while `throughput.csv` is better for port-level link utilization evidence.
- Before saying whether the topology approaches line rate, pick which metric you mean: per-port Rx/Tx or per-task throughput.

## Unsafe Wording

- `throughput.csv` directly proves end-to-end line-rate efficiency for the whole workload.
- `task_statistics.csv` and `throughput.csv` are interchangeable throughput views.
- the current helper already knows the measured line-rate ratio.

## Authority

- `code/doc fact`: `scratch/ns-3-ub-tools/trace_analysis/cal_throughput.py` writes per-port `Rx/Tx throughput(Gbps)`
- `code/doc fact`: `scratch/ns-3-ub-tools/trace_analysis/task_statistics.py` writes per-task `taskThroughput(Gbps)`
- `code/doc fact`: no helper-local script under ` .codex/skills/openusim-helper/scripts/ ` currently computes a line-rate ratio from measured throughput
