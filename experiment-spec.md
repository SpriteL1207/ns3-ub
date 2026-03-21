# Experiment Spec

## Goal
- 在 2DFM 4×4 拓扑上运行 Ring AllReduce，观察集合通信吞吐

## Topology
- family: nd-full-mesh
- row_num: 4, col_num: 4, host_num: 16
- bandwidth: 400Gbps, delay: 10ns, forward_delay: 1ns

## Workload
- algo: ar_ring
- host_count: 16
- comm_domain_size: 16
- data_size: 16MB
- rank_mapping: linear (default)
- phase_delay: 0 (default)

## Network Overrides
- 无（使用默认值）

## Observability
- balanced（task_statistics.csv + throughput.csv）

## Startup Readiness
- ./ns3 存在
- scratch/ns-3-ub-tools/ 存在（含 net_sim_builder.py、build_traffic.py、parse_trace.py）
- build/ 和 cmake-cache/ 存在
- scratch/2nodes_single-tp 存在

## Execution Record
- case path: scratch/2dfm4x4-ar_ring
- run command: ./ns3 run 'scratch/ub-quick-example scratch/2dfm4x4-ar_ring'
- output artifacts:
  - scratch/2dfm4x4-ar_ring/output/task_statistics.csv（480 tasks）
  - scratch/2dfm4x4-ar_ring/output/throughput.csv（96 port-directions）
- wall-clock: ~13s（config 373us / run 11.25s / trace 1.62s）
- status: completed, no errors

## Analysis Notes
- （待填写）
