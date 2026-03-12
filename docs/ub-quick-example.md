# `ub-quick-example` 入口文档

`ub-quick-example` 是当前 unified-bus 配置驱动场景的统一用户入口。

它的定位有两层：
- 作为可直接运行的 config-driven 示例入口
- 作为 unified-bus 在 ns-3 中的建模/运行步骤说明

## 当前能力边界

| 模式 | 状态 | 说明 |
|---|---|---|
| 本地单进程单线程 | 支持 | 默认模式 |
| 本地单进程 MTP 多线程 | 支持 | 使用 `--mtp-threads=<N>`，`N>=2` |
| MPI 多进程 + `traffic.csv` / `UbTrafficGen` | 不支持 | 纯 MPI 与 MPI+MTP 都会显式 fail-fast |
| `ub-mtp-remote-tp-regression` | 仅回归用途 | 不是用户入口 |

当前分支里，统一-bus 的多进程数据路径适配已经具备，但 `UbTrafficGen` 仍被定义为**进程内 traffic generator**，不提供跨 rank 的 DAG / operator 同步。因此：

- `ub-quick-example` 可以作为本地 config-driven 用户入口
- `ub-quick-example` 在 MPI 多进程下会显式拒绝 `traffic.csv` 场景

## 构建

最小本地入口：

```bash
cmake --build ./cmake-cache -j 7 --target ub-quick-example
```

如果要跑本地 MTP：

```bash
./ns3 configure --enable-mtp --enable-examples
cmake --build ./cmake-cache -j 7 --target ub-quick-example
```

如果只是为了验证当前纯 MPI fail-fast 边界：

```bash
./ns3 configure --enable-mpi --enable-examples
cmake --build ./cmake-cache -j 7 --target ub-quick-example
```

如果还要验证 MPI+MTP hybrid fail-fast 边界：

```bash
./ns3 configure --enable-mpi --enable-mtp --enable-examples
cmake --build ./cmake-cache -j 7 --target ub-quick-example
```

## 运行命令

本地单进程：

```bash
build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-local-hybrid-minimal
```

本地 MTP 多线程：

```bash
build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-local-hybrid-minimal --mtp-threads=2
```

测试模式：

```bash
build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-local-hybrid-minimal --test
```

当前分支下，下面这些 MPI 多进程命令**都预期会被拒绝**：

```bash
mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-mpi-hybrid-minimal --test
```

```bash
mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-mpi-hybrid-minimal --mtp-threads=2 --test
```

## `case-path` 目录

一个最小 case 目录通常包含：

- `network_attribute.txt`
- `node.csv`
- `topology.csv`
- `routing_table.csv`
- `traffic.csv`

可选：

- `transport_channel.csv`
- `fault.csv`

更完整的字段定义、格式约束、工具链生成方式见： `scratch/README.md`

## 入口执行顺序

`ub-quick-example` 按如下顺序执行：

1. 解析 CLI
2. 校验 `case-path`
3. 选择 runtime 模式（本地单线程 / 本地 MTP）
4. 加载 `network_attribute.txt`
5. 创建 node / topo / routing / TP
6. 加载 `traffic.csv` 并初始化 `UbTrafficGen`
7. 激活 `UbApp`
8. 运行仿真
9. 解析 trace 并输出 wall-clock

这条路径对应的核心实现位于： `src/unified-bus/examples/ub-quick-example.cc`

## 常见问题

### 1. `missing required case path`

没有提供 `--case-path`，或者没有给 positional `casePath`。

### 2. `missing required case file`

`case-path` 下缺少必需配置文件。

### 3. `UbTrafficGen does not support MPI multi-process usage in this branch`

这是当前分支的预期行为，不是误报。

含义是：
- unified-bus 多进程数据路径已适配
- 但 `traffic.csv` / `UbTrafficGen` 仍不支持多进程分布式 DAG 同步

### 4. trace 没有后处理结果

检查：
- `UB_PARSE_TRACE_ENABLE`
- `UB_PYTHON_SCRIPT_PATH`

## 相关文档

- 项目总览： `README.md`
- 快速开始： `QUICK_START.md`
- Unison / MPI / MTP 背景： `UNISON_README.md`
- case 配置格式： `scratch/README.md`
- 阶段报告： `docs/reports/2026-03-11-ub-quick-example-mpi-unification-report.md`
