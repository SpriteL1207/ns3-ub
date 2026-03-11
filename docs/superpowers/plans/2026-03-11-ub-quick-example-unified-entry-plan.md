# UB Quick Example Unified Entry Implementation Plan

> **For agentic workers:** REQUIRED: Use superpowers:subagent-driven-development (if subagents available) or superpowers:executing-plans to implement this plan. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 把 `ub-quick-example` 变成 unified-bus 的统一 config-driven 正式入口，并在不破坏现有 MPI 回归的前提下新增原生多进程 / 多进程+多线程能力。

**Architecture:** 抽出一个公共 config-driven runner，收敛 `ub-quick-example` 与 `ub-mpi-config-smoke` 的初始化、构图和按 rank 激活任务逻辑；`ub-quick-example` 保留用户界面能力，`ub-mpi-config-smoke` 只保留测试校验能力。实现过程中优先修正初始化顺序与职责边界，不做无关性能工程。

**Tech Stack:** ns-3 C++, unified-bus, MPI (`MpiInterface`), MTP (`MtpInterface`), existing `ub-mpi-config-smoke` regressions, config-driven scratch cases.

---

## File Map

- Modify: `src/unified-bus/examples/ub-quick-example.cc`
- Modify: `src/unified-bus/examples/ub-mpi-config-smoke.cc`
- Create: `src/unified-bus/examples/ub-config-runner.h`
- Create: `src/unified-bus/examples/ub-config-runner.cc`
- Modify: `src/unified-bus/examples/CMakeLists.txt`
- Verify with existing cases under `scratch/`
- Optional test fixture update: `scratch/ub-local-hybrid-minimal/`

## Chunk 1: 抽公共 runner

### Task 1: 定义公共 runner API

**Files:**
- Create: `src/unified-bus/examples/ub-config-runner.h`
- Create: `src/unified-bus/examples/ub-config-runner.cc`
- Modify: `src/unified-bus/examples/CMakeLists.txt`

- [ ] **Step 1: 梳理现有共同行为**

Read:
- `src/unified-bus/examples/ub-quick-example.cc`
- `src/unified-bus/examples/ub-mpi-config-smoke.cc`

记录需要收敛的公共流程：
- 参数到运行模式的映射
- `SetComponentsAttribute/CreateNode/CreateTopo/AddRoutingTable/CreateTp`
- traffic 读取与任务激活
- MPI / MTP 初始化与收尾

- [ ] **Step 2: 先写最小 runner 头文件**

在 `src/unified-bus/examples/ub-config-runner.h` 定义最小 API，例如：

```cpp
struct UbConfigRunOptions
{
    std::string casePath;
    uint32_t mtpThreads;
    uint32_t stopMs;
    bool enableMpi;
};

struct UbConfigRunResult
{
    bool localPassed;
    bool globalPassed;
    uint32_t localTaskCount;
    uint32_t mpiRank;
    uint32_t mpiSize;
    bool mpiEnabled;
};
```

- [ ] **Step 3: 在 `.cc` 中落最小骨架**

实现最小公共函数：

- 初始化运行模式
- 构图
- 激活本地 task
- 运行模拟
- 收尾

先不接测试专有 hook。

- [ ] **Step 4: 编译检查**

Run:
`cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target ub-quick-example ub-mpi-config-smoke`

Expected:
- PASS

- [ ] **Step 5: Commit**

```bash
git add src/unified-bus/examples/ub-config-runner.h src/unified-bus/examples/ub-config-runner.cc src/unified-bus/examples/CMakeLists.txt
git commit -m "refactor: add shared ub config runner"
```

## Chunk 2: 让 `ub-quick-example` 接入统一 runner

### Task 2: 修正 `ub-quick-example` 初始化顺序并接入 runner

**Files:**
- Modify: `src/unified-bus/examples/ub-quick-example.cc`
- Test via: `scratch/ub-local-hybrid-minimal/`

- [ ] **Step 1: 删除 `ub-quick-example` 内部重复 runner 逻辑**

删除或迁移：
- `ConfigCase(...)`
- `InitiateTasks(...)` 中与公共 runner 重复的核心逻辑

保留：
- 进度打印
- wall-clock
- trace/parse
- 用户 CLI

- [ ] **Step 2: 增加统一 CLI**

确保 `ub-quick-example` 支持：

- `--case-path`
- `--mtp-threads`
- `--stop-ms`
- 兼容一个 positional case path

- [ ] **Step 3: 接入正确的模式初始化**

要求：
- 不在 `MPI_Init` 前触发 MPI 相关路径
- 在 MPI world size > 1 时自动走 MPI 路径
- `mtpThreads <= 1` 时不启用 MTP

- [ ] **Step 4: 先做本地 smoke**

Run:
`build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-local-hybrid-minimal`

Expected:
- PASS

- [ ] **Step 5: 做本地多线程 smoke**

Run:
`build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-local-hybrid-minimal --mtp-threads=2`

Expected:
- 不再出现 `MPI_Testany() ... before MPI_INIT`

- [ ] **Step 6: Commit**

```bash
git add src/unified-bus/examples/ub-quick-example.cc
git commit -m "feat: make ub-quick-example use shared runner"
```

## Chunk 3: 让 `ub-quick-example` 原生支持 MPI / MPI+MTP

### Task 3: 打通 `ub-quick-example` 的两进程和两进程两线程路径

**Files:**
- Modify: `src/unified-bus/examples/ub-config-runner.cc`
- Modify: `src/unified-bus/examples/ub-quick-example.cc`

- [ ] **Step 1: 把 MPI 模式下的本地 task 激活规则搬入 runner**

规则：
- 只激活本 rank 拥有的 source node task
- 复用已有 ownership helper

- [ ] **Step 2: 实现全局完成与退出语义**

要求：
- MPI 模式下每个 rank 只基于本地 task 判定 local complete
- 通过 MPI 聚合得到 global complete

- [ ] **Step 3: 运行两进程 smoke**

Run:
`mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-mpi-hybrid-minimal`

Expected:
- PASS

- [ ] **Step 4: 运行两进程两线程 smoke**

Run:
`mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-mpi-hybrid-minimal --mtp-threads=2`

Expected:
- PASS

- [ ] **Step 5: Commit**

```bash
git add src/unified-bus/examples/ub-config-runner.cc src/unified-bus/examples/ub-quick-example.cc
git commit -m "feat: add mpi and hybrid support to ub-quick-example"
```

## Chunk 4: 把 `ub-mpi-config-smoke` 收缩成测试壳

### Task 4: 让 `ub-mpi-config-smoke` 基于公共 runner 运行

**Files:**
- Modify: `src/unified-bus/examples/ub-mpi-config-smoke.cc`

- [ ] **Step 1: 保留测试专有 flag**

保留：
- `--test`
- `--verify-tp-ownership`
- `--verify-packed-systemid`
- `--verify-cbfc-control`

- [ ] **Step 2: 删除重复的主流程代码**

迁移或删除：
- 重复的 config 构图主流程
- 重复的 task 激活逻辑
- 重复的 MPI / MTP 初始化主流程

- [ ] **Step 3: 只在 runner 暴露的 hook 上做验证**

要求：
- `verify-cbfc-control` 继续基于真实 flow-control restore 路径
- 不重新引入 link 层手工语义 oracle

- [ ] **Step 4: 编译并跑最小 smoke**

Run:
`python3 ./ns3 run ub-mpi-config-smoke --no-build --command-template "mpirun -np 2 %s --test --case-path=scratch/ub-mpi-hybrid-minimal --mtp-threads=2 --verify-packed-systemid --verify-tp-ownership --stop-ms=50"`

Expected:
- PASS

- [ ] **Step 5: Commit**

```bash
git add src/unified-bus/examples/ub-mpi-config-smoke.cc
git commit -m "refactor: make ub-mpi-config-smoke use shared runner"
```

## Chunk 5: 回归与交付

### Task 5: 重跑现有 MPI 回归矩阵

**Files:**
- Verify only

- [ ] **Step 1: Rebuild**

Run:
`cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target unified-bus-test ub-quick-example ub-mpi-config-smoke ub-hybrid-smoke mpi-test`

- [ ] **Step 2: 跑 unified-bus suite**

Run:
`build/utils/ns3.44-test-runner-default --suite=unified-bus --verbose`

- [ ] **Step 3: 跑现有 MPI suite**

Run:
```bash
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-smoke-2 --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-smoke-2 --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-cbfc-2 --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-ldst-2 --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-multi-remote-2 --verbose
build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mpi-config-hybrid-cbfc-multivl-2 --verbose
```

- [ ] **Step 4: 跑 `ub-quick-example` 统一入口矩阵**

Run:
```bash
build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-local-hybrid-minimal
build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-local-hybrid-minimal --mtp-threads=2
mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-mpi-hybrid-minimal
mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=scratch/ub-mpi-hybrid-minimal --mtp-threads=2
```

Expected:
- 全部 PASS

- [ ] **Step 5: 更新文档并提交**

更新：
- `docs/plans/2026-03-10-ub-native-mpi-ownership-plan.md`

Commit:

```bash
git add docs/plans/2026-03-10-ub-native-mpi-ownership-plan.md
git commit -m "docs: record unified ub entry verification"
```

## Chunk 6: 收尾判断

### Task 6: 给出是否可替换旧入口的结论

**Files:**
- Modify: `docs/plans/2026-03-10-ub-native-mpi-ownership-plan.md`

- [ ] **Step 1: 记录已证明能力**

至少包含：
- `ub-quick-example` 本地单进程
- `ub-quick-example` 本地多线程
- `ub-quick-example` 两进程
- `ub-quick-example` 两进程两线程

- [ ] **Step 2: 记录仍未覆盖项**

仅记录事实，例如：
- 更大规模 rank 矩阵仍未覆盖
- benchmark 仍非 gate

- [ ] **Step 3: 明确替换建议**

二选一：
- `ub-quick-example` can replace `ub-mpi-config-smoke` as user-facing entry
- `not yet`

- [ ] **Step 4: Commit**

```bash
git add docs/plans/2026-03-10-ub-native-mpi-ownership-plan.md
git commit -m "docs: conclude unified ub entry readiness"
```
