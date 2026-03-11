# UB Quick Example Unified Entry Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 让 `ub-quick-example` 成为 unified-bus 配置驱动场景的唯一用户入口，同时把 `ub-mpi-config-smoke` 降级为测试过渡壳并逐步下沉其验证职责。

**Architecture:** 取消当前“厚 `ub-config-runner` + 薄 `ub-quick-example`”的结构，改回由 `ub-quick-example` 显式展示 ns-3/UB 的建模与运行步骤：选择 simulator 模式、加载配置、建图、激活 traffic、运行仿真、解析 trace。测试职责拆成两类：ownership / builder / preload 等白盒逻辑下沉到 `src/unified-bus/test/ub-test.cc`，端到端 MPI/MTP/hybrid 场景改为 system/example test 直接拉 `ub-quick-example`。`ub-mpi-config-smoke` 在过渡期只保留最小测试壳，待回归全部迁走后删除。

**Tech Stack:** ns-3 C++, unified-bus, MPI, MTP, `ExampleAsTestCase`, `MpiTestSuite`, unified-bus unit/system tests.

---

## 旧计划状态核对

- `docs/plans/2026-03-10-ub-native-mpi-ownership-plan.md` 的 **Phase 1-5** 已基本完成并已有 checkpoint commit 支撑。
- 该计划里 **未完成 / 不再适用** 的部分主要是：
  - `quick-example` 仍未成为 formal native-MPI 统一入口；
  - `ub-mpi-config-smoke` 仍承担过多产品入口职责；
  - Phase 6/7 的“主分支就绪 / 置信度扩展”尚未按新的架构定位完成。
- 本计划**替代并收敛**上述未完成部分；后续不再沿“继续强化 `ub-config-runner` / `ub-mpi-config-smoke`”路线推进。

## 文件职责先定稿

- `src/unified-bus/examples/ub-quick-example.cc`
  - 统一用户入口
  - 显式示范 UB on ns-3 的建模与运行步骤
  - 保留 CLI / progress / trace / wall-clock
- `src/unified-bus/model/ub-utils.h`
  - 现有配置驱动底层能力声明（不新增“总控 runner”）
- `src/unified-bus/model/ub-utils.cc`
  - 继续承载现有的 case-file -> UB object graph 装配能力
  - 仅做必要的小修正，不引入新总控抽象
- `src/unified-bus/examples/ub-mpi-config-smoke.cc`
  - 过渡期测试壳
  - 不再新增产品逻辑
- `src/unified-bus/test/ub-test.cc`
  - ownership / rank helper / builder / preload / activation 等白盒测试
- `src/mpi/test/mpi-test-suite.cc`
  - MPI black-box 回归入口
  - 后续改为优先拉 `ub-quick-example`
- `src/unified-bus/examples/CMakeLists.txt`
  - example 构建清单；最终应去掉不再需要的 smoke/example 入口

## Chunk 1: 回填 `ub-quick-example` 为显式步骤入口

### Task 1: 去掉 `ub-quick-example` 对厚 runner 的依赖

**Files:**
- Modify: `src/unified-bus/examples/ub-quick-example.cc`
- Read: `src/unified-bus/model/ub-utils.h`
- Read: `src/unified-bus/model/ub-utils.cc`
- Read: `src/unified-bus/examples/ub-config-runner.h`
- Read: `src/unified-bus/examples/ub-config-runner.cc`

- [ ] **Step 1: 先写出回填后的步骤顺序清单**

在 `src/unified-bus/examples/ub-quick-example.cc` 对照现有 `ub-utils` API，明确 main 中必须显式出现的步骤：

1. `PrepareSimulatorMode(...)`
2. `SetComponentsAttribute(...)`
3. `CreateTraceDir()`
4. `CreateNode(...)`
5. `CreateTopo(...)`
6. `AddRoutingTable(...)`
7. `CreateTp(...)`
8. `TopoTraceConnect()`
9. `ActivateTasks(...)`
10. `Simulator::Run()`
11. `UbUtils::Destroy()`
12. `Simulator::Destroy()`
13. `ParseTrace()`

- [ ] **Step 2: 删除 `RunUbConfigCase(...)` 调用，回填显式步骤**

在 `src/unified-bus/examples/ub-quick-example.cc`：

- 移除 `#include "ub-config-runner.h"`
- 用本文件内局部函数回填：
  - simulator 模式准备
  - case 配置步骤串接
  - task 激活
- 保持用户能直接看到 UB 的 API 使用顺序

- [ ] **Step 3: 保持本地 / MTP / MPI / hybrid 四种模式的 CLI 行为**

在 `src/unified-bus/examples/ub-quick-example.cc` 保留：

- `--case-path`
- `--mtp-threads`
- `--stop-ms`

并明确：

- 本地：默认 simulator
- 本地 MTP：`MultithreadedSimulatorImpl`
- MPI：`DistributedSimulatorImpl`
- MTP + MPI：hybrid 路径

- [ ] **Step 4: 修正 trace 目录初始化顺序**

保证 `UbUtils::CreateTraceDir()` 发生在 `SetComponentsAttribute(...)` 之后，避免依赖未设置的 `g_config_path`。

- [ ] **Step 5: 验证 `ub-quick-example` 本地路径**

Run:

```bash
cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target ub-quick-example unified-bus-test
build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-local-hybrid-minimal
build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-local-hybrid-minimal --mtp-threads=2
```

Expected:

- 能编译
- 本地单线程 / 多线程都能跑通
- 输出仍保留进度与 wall-clock

- [ ] **Step 6: Commit**

```bash
git add src/unified-bus/examples/ub-quick-example.cc
git commit -m "refactor: make ub-quick-example explicit config entry"
```

## Chunk 2: 收缩并降级 `ub-mpi-config-smoke`

### Task 2: 明确 smoke 降级时机并先做“冻结”

**Files:**
- Modify: `src/unified-bus/examples/ub-mpi-config-smoke.cc`
- Modify: `src/unified-bus/examples/CMakeLists.txt`
- Read: `src/mpi/test/mpi-test-suite.cc`

- [ ] **Step 1: 冻结 `ub-mpi-config-smoke` 职责**

在文件头部注释或邻近说明中明确：

- 这是过渡测试壳
- 不再作为 unified-bus 用户入口
- 后续不再往里加新能力

- [ ] **Step 2: 记录 smoke 降级时序**

执行策略固定为：

1. 先完成 `ub-quick-example` 统一入口回填；
2. 再迁移 black-box 回归到 `ub-quick-example`；
3. 最后才删除或极限收缩 `ub-mpi-config-smoke`。

也就是说，**smoke 降级从本计划 Chunk 2 开始，真正删除在 Chunk 5**。

- [ ] **Step 3: 检查是否还需要保留独立 example 构建**

如果当前 `mpi-test-suite` 还直接依赖 `ub-mpi-config-smoke`，先保留构建目标；暂不删除二进制。

- [ ] **Step 4: Commit**

```bash
git add src/unified-bus/examples/ub-mpi-config-smoke.cc src/unified-bus/examples/CMakeLists.txt
git commit -m "chore: freeze ub-mpi-config-smoke as test-only shell"
```

## Chunk 3: 白盒验证下沉到 `src/unified-bus/test/`

### Task 3: 补全 ownership / activation / preload 的白盒单测

**Files:**
- Modify: `src/unified-bus/test/ub-test.cc`
- Read: `src/unified-bus/model/ub-utils.h`
- Read: `src/unified-bus/model/ub-utils.cc`
- Read: `src/unified-bus/examples/ub-mpi-config-smoke.cc`

- [ ] **Step 1: 列出从 smoke 迁出的白盒项**

仅迁移这些：

- `ExtractMpiRank`
- `IsSameMpiRank`
- `IsNodeOwnedByCurrentRank`
- remote/local link builder 决策
- TP preload ownership
- task activation ownership 过滤

- [ ] **Step 2: 为 task activation ownership 写失败测试**

在 `src/unified-bus/test/ub-test.cc` 新增 focused test，直接断言：

- MPI 模式只激活本 rank 拥有的 source task
- remote source node 不会重复创建本地 `UbApp`

- [ ] **Step 3: 写最小实现或抽出最小可测局部函数**

如果 `ub-quick-example.cc` 内局部函数无法直接测，允许抽出**最小**可测函数，但不要重新引入厚 runner。

- [ ] **Step 4: 跑白盒测试**

Run:

```bash
cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target unified-bus-test
build/utils/ns3.44-test-runner-default --suite=unified-bus --verbose
```

Expected:

- 现有 unified-bus 单测继续通过
- 新增 ownership / activation 白盒测试通过

- [ ] **Step 5: Commit**

```bash
git add src/unified-bus/test/ub-test.cc src/unified-bus/examples/ub-quick-example.cc
git commit -m "test: move ub mpi ownership checks into unified-bus tests"
```

## Chunk 4: 黑盒回归切到 `ub-quick-example`

### Task 4: 用 `ub-quick-example` 替代 smoke 作为 black-box MPI/MTP 回归入口

**Files:**
- Modify: `src/mpi/test/mpi-test-suite.cc`
- Modify: `src/unified-bus/test/ub-test.cc`
- Read: `src/unified-bus/examples/ub-quick-example.cc`
- Read: `src/unified-bus/examples/ub-mpi-config-smoke.cc`

- [ ] **Step 1: 先选最小黑盒矩阵**

首批只迁移这些 case 到 `ub-quick-example`：

- MPI baseline：`scratch/ub-mpi-hybrid-minimal`
- MPI + MTP baseline：`scratch/ub-mpi-hybrid-minimal --mtp-threads=2`
- LDST smoke：`scratch/ub-mpi-hybrid-ldst-minimal --mtp-threads=2`
- multi-remote smoke：`scratch/ub-mpi-hybrid-multi-remote --mtp-threads=2`

CBFC 作为额外 integration case，后置到黑盒矩阵稳定后再迁。

- [ ] **Step 2: 为 `ub-quick-example` 新增或改造 system test 入口**

优先用 `ExampleAsTestCase` / `MpiTestSuite` 直接跑：

```bash
python3 ./ns3 run ub-quick-example --no-build --command-template="mpirun -np 2 %s --case-path=<case> [--mtp-threads=2]"
```

- [ ] **Step 3: 先跑最小黑盒矩阵**

Run:

```bash
mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-mpi-hybrid-minimal
mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-mpi-hybrid-minimal --mtp-threads=2
mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-mpi-hybrid-ldst-minimal --mtp-threads=2
mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-mpi-hybrid-multi-remote --mtp-threads=2
```

Expected:

- 2 进程路径稳定通过
- `ub-quick-example` 已能覆盖之前 smoke 的主干场景

- [ ] **Step 4: Commit**

```bash
git add src/mpi/test/mpi-test-suite.cc src/unified-bus/test/ub-test.cc
git commit -m "test: run ub quick example as mpi black-box entry"
```

## Chunk 5: 删除厚 runner，完成 smoke 降级

### Task 5: 删除 `ub-config-runner`，最小化或删除 `ub-mpi-config-smoke`

**Files:**
- Delete: `src/unified-bus/examples/ub-config-runner.h`
- Delete: `src/unified-bus/examples/ub-config-runner.cc`
- Modify or Delete: `src/unified-bus/examples/ub-mpi-config-smoke.cc`
- Modify: `src/unified-bus/examples/CMakeLists.txt`
- Modify: `src/mpi/test/mpi-test-suite.cc`

- [ ] **Step 1: 删除已不再使用的 `ub-config-runner`**

在确认 `ub-quick-example` 与测试均不再依赖后，移除 runner 文件与构建接线。

- [ ] **Step 2: 评估 `ub-mpi-config-smoke` 的最终处理**

按下面顺序判断：

1. 如果所有 black-box MPI 回归已切到 `ub-quick-example`，则删除 `ub-mpi-config-smoke`；
2. 如果仍有 1-2 条短期过渡用例依赖它，则把它收缩成极薄 wrapper，并在计划执行结束前列出删除条件。

- [ ] **Step 3: 跑完整回归**

Run:

```bash
cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target ub-quick-example unified-bus-test mpi-test
build/utils/ns3.44-test-runner-default --suite=unified-bus --verbose
build/utils/ns3.44-test-runner-default --suite=unified-bus-examples --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-smoke-2 --verbose
```

若 `mpi-example-ub-mpi-config-smoke-2` 已被替换，则改跑新的 `ub-quick-example` MPI suites。

- [ ] **Step 4: Commit**

```bash
git add src/unified-bus/examples/CMakeLists.txt src/mpi/test/mpi-test-suite.cc src/unified-bus/examples/ub-quick-example.cc src/unified-bus/test/ub-test.cc
git rm -f src/unified-bus/examples/ub-config-runner.h src/unified-bus/examples/ub-config-runner.cc
git commit -m "refactor: unify ub mpi entry on ub-quick-example"
```

## Chunk 6: 文档与交付说明

### Task 6: 更新用户入口和测试入口说明

**Files:**
- Modify: `UNISON_README.md`
- Modify: `docs/workflows/` 下与 MPI / quick example 相关的文档（如存在）
- Create or Modify: 本轮报告文档路径（执行时确定）

- [ ] **Step 1: 更新用户命令**

明确只推荐：

```bash
build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=<case>
mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=<case> --mtp-threads=2
```

- [ ] **Step 2: 更新测试说明**

说明：

- `ub-quick-example` 是统一入口
- smoke 仅为测试过渡或已删除
- 白盒验证在哪些 suite
- 黑盒 MPI 回归在哪些 suite

- [ ] **Step 3: Commit**

```bash
git add UNISON_README.md docs/workflows
git commit -m "docs: describe ub quick example as unified mpi entry"
```

## 验收标准

- `ub-quick-example` 不再依赖厚 `ub-config-runner`
- `ub-quick-example` 明确展示 UB 在 ns-3 中的建模与运行步骤
- 用户只需要记住 `ub-quick-example` 这一条入口
- 白盒 MPI ownership / preload / activation 验证位于 `src/unified-bus/test/`
- 黑盒 MPI/MTP/hybrid 回归优先直接运行 `ub-quick-example`
- `ub-mpi-config-smoke` 已降级为测试过渡壳，或已删除

## 备注

- smoke 降级**不是最后才想起来做**；从本计划 **Chunk 2** 就开始冻结，并在 **Chunk 5** 完成真正的下沉/删除。
- 本计划不扩展新的性能 benchmark，也不在这一轮做大规模 case 矩阵膨胀。
- 若执行中发现 `ExampleAsTestCase` 的 sandbox 假失败问题仍阻碍 MPI gate，优先修 test harness 可观测性，不回退到继续强化 `ub-mpi-config-smoke`。
