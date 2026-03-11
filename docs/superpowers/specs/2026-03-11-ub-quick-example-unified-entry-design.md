# UB Quick Example Unified Entry Design

**目标**

把 `src/unified-bus/examples/ub-quick-example.cc` 收敛成 unified-bus 的统一 config-driven 用户入口，在保持现有本地单进程能力的同时，新增原生多进程 / 多进程+多线程能力；`src/unified-bus/examples/ub-mpi-config-smoke.cc` 退化为测试包装器，只承载测试特有校验。

**背景**

- 当前 `ub-quick-example` 有用户界面、进度输出、wall-clock 统计和 trace/parse，但不具备正确的 MPI 初始化路径。
- 当前 `ub-mpi-config-smoke` 具备正确的 native MPI / MTP+MPI 初始化与按 rank 激活任务逻辑，但它是测试入口，不适合作为正式用户界面。
- 已验证通过的 config-driven native MPI 路径包括：minimal hybrid `TP`、`CBFC`、`LDST`、multi-remote、single-remote-edge multi-priority `CBFC`。

## 设计结论

采用“公共 runner + 双前端”的结构：

- `ub-quick-example`：正式用户入口
- `ub-mpi-config-smoke`：测试壳
- 新增一个公共 runner 模块：负责 config-driven case 的构建、模式初始化、按 rank 激活任务、完成判定、以及测试所需的最小 hook

不直接把 `ub-mpi-config-smoke` 改名为正式入口，也不在 `ub-quick-example` 中复制一份 MPI 路径。

## 方案比较

### 方案 A：抽公共 runner，统一两条入口

**做法**

- 从 `ub-mpi-config-smoke` 与 `ub-quick-example` 抽出公共执行骨架
- `ub-quick-example` 负责 CLI、日志、进度、trace/parse、wall-clock
- `ub-mpi-config-smoke` 只保留 `--test` 与验证 flag

**优点**

- 只有一套 config-driven 运行路径
- MPI / MTP 初始化规则不会再漂
- 后续扩展 `CBFC`、`LDST`、multi-remote 都沿用同一路径

**缺点**

- 需要中等规模整理

**结论**

推荐。

### 方案 B：直接把 MPI 逻辑抄进 `ub-quick-example`

**优点**

- 初看实现快

**缺点**

- 立刻形成第二套 runner
- 后续测试与正式入口很容易漂移

**结论**

不推荐。

### 方案 C：继续以 `ub-mpi-config-smoke` 为主入口

**优点**

- 代码改动最小

**缺点**

- 测试程序承担正式入口职责，边界不清
- 用户界面能力继续分裂

**结论**

只适合作为短期过渡，不适合作为目标形态。

## 架构设计

### 1. 公共 runner 的职责

公共 runner 负责以下固定流程：

1. 解析运行模式输入：
   - local single-process
   - local multi-thread
   - MPI single-thread
   - MPI multi-thread
2. 正确初始化：
   - 若 `--mtp-threads > 1`，先 `MtpInterface::Enable(...)`
   - 若处于 MPI 模式，再 `MpiInterface::Enable(...)`
   - 非 MTP 的 MPI 模式继续保留 distributed simulator 语义
3. 从 config 构图：
   - `SetComponentsAttribute`
   - `CreateNode`
   - `CreateTopo`
   - `AddRoutingTable`
   - `CreateTp`
4. 根据 ownership / current rank 激活本地 source task
5. 运行模拟并提供统一结果对象

runner 不负责：

- 用户友好的进度展示
- trace 目录创建与 trace parse 策略
- 测试 oracle 的业务判断

### 2. `ub-quick-example` 的职责

`ub-quick-example` 只做用户层需求：

- CLI：
  - `--case-path`
  - `--mtp-threads`
  - 可选 `--stop-ms`
- 打印当前模式：
  - local
  - local+mtp
  - mpi
  - mpi+mtp
- 在允许时展示进度
- 输出 wall-clock
- 控制 trace/parse

它不再自己决定“哪些 task 在本 rank 激活”，不再自己维护第二套 MPI 初始化逻辑。

### 3. `ub-mpi-config-smoke` 的职责

`ub-mpi-config-smoke` 退化为测试包装器：

- 调用公共 runner
- 追加测试专有校验：
  - `--verify-packed-systemid`
  - `--verify-tp-ownership`
  - `--verify-cbfc-control`
- 在 rank 0 输出 `TEST : 00000 : PASSED/FAILED`

它不再承载正式用户界面职责。

## CLI 设计

### `ub-quick-example`

推荐最终支持：

- `./ns3 run ub-quick-example -- --case-path=scratch/ub-local-hybrid-minimal`
- `./ns3 run ub-quick-example -- --case-path=scratch/ub-local-hybrid-minimal --mtp-threads=2`
- `mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-mpi-hybrid-minimal`
- `mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-mpi-hybrid-minimal --mtp-threads=2`

其中：

- 若检测到 MPI world size > 1，则自动走 MPI 路径
- 若 `--mtp-threads > 1`，则自动走 MTP 路径

## 数据与控制流

### local 路径

1. `ub-quick-example` 解析参数
2. runner 初始化 local 模式
3. 构图
4. 激活全部 task
5. 运行模拟
6. `ub-quick-example` 处理进度、trace、wall-clock

### MPI 路径

1. `mpirun` 启动多个进程
2. `ub-quick-example` 解析参数
3. runner 初始化 MPI / MTP
4. 构图
5. 每个 rank 只激活本 rank 所拥有 source node 的 task
6. 全局完成后退出
7. `ub-quick-example` 只在合适的 rank 做用户输出汇总

## 错误处理

必须 fail-fast 的情况：

- MPI 模式下 world size 与入口要求不匹配
- `--mtp-threads > 1` 但未编译 `NS3_MTP`
- case 文件缺失
- 配置解析失败

必须避免的错误：

- 在 `MPI_Init` 前触发任何依赖 MPI 的 MTP 路径
- local 入口与 smoke 入口对同一 case 激活不同任务集合
- 把测试 oracle 混进正式入口主逻辑

## 测试与验收

### 功能验收

至少需要覆盖：

1. `ub-quick-example` 本地单进程
2. `ub-quick-example` 本地多线程
3. `ub-quick-example` 两进程单线程
4. `ub-quick-example` 两进程两线程

### 回归保护

必须保证现有 `ub-mpi-config-smoke` 回归不退化：

- minimal hybrid `TP`
- hybrid `CBFC`
- hybrid `LDST`
- multi-remote
- single-remote-edge multi-priority `CBFC`

### 非目标

本轮不做：

- 新 benchmark 框架
- 吞吐/时延 gate
- 更大规模多 rank 矩阵
- `fat-tree-hybrid` 与 unified-bus 的接口统一

## 风险与失效条件

### 风险 1：`ub-quick-example` 仍保留旧的本地任务激活逻辑

后果：

- local 与 MPI 路径行为不一致

规避：

- 任务激活逻辑必须只保留在公共 runner 一处

### 风险 2：把测试 flag 设计进正式入口

后果：

- 正式入口被测试语义污染

规避：

- 让 `ub-mpi-config-smoke` 持有测试特有选项，runner 只暴露最小 hook

### 风险 3：为了快速接通，继续复制 `ConfigureCase` / `InitiateTasks`

后果：

- 未来再出第三套逻辑

规避：

- 第一轮重构就把 config-driven 主流程收敛到一个模块

## 推荐实施顺序

1. 先抽公共 runner
2. 先让 `ub-quick-example` 接到 runner 上并打通 `MPI/MTP`
3. 再把 `ub-mpi-config-smoke` 改成基于 runner 的测试壳
4. 最后补 `ub-quick-example` 入口的本地 / MPI / MTP 验证矩阵
