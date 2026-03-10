# UB MPI 多进程接入进展报告（2026-03-09）

## 结论

- [事实] 当前分支已经打通一条 **MPI-only、2 rank、静态预配置 TP** 的 unified-bus 多进程冒烟链路。
- [事实] 已验证通过：
  - `unified-bus` unit suite
  - `ub-mpi-config-smoke` 2-rank smoke
- [事实] 当前能力边界仍然是：
  - **已支持**：跨 rank remote link + 预配置 TP 的收发路径
  - **未完成**：跨 MPI rank 的 **按需/动态 TP 创建**
  - **未完成**：`MTP + MPI` hybrid packed-systemId 专用 smoke

## 本次根因与修复

### 1. remote link builder
- [事实] `CreateTopo(...)` 现在会根据两端节点的 `systemId` 所属 MPI rank，选择创建 `UbLink` 或 `UbRemoteLink`。
- [事实] 在 `NS3_MTP` 条件下，remote 判定比较的是 `systemId` 低 16 位 rank，而不是完整 `systemId`。

### 2. MPI 收包反序列化崩溃
- [事实] 之前 2-rank 运行时的 `TypeId::LookupByHash` 崩溃，根因是 `UbFlowTag` / `UbPacketTraceTag` 未在模块级确保注册。
- [事实] 已新增 `ub-tag.cc` 做 `NS_OBJECT_ENSURE_REGISTERED(...)`，该崩溃已消失。

### 3. TP 缺失导致远端收包断言
- [事实] `UbSwitch::HandleURMADataPacket()` 在远端按 `dstTpn` 查 TP 时，曾触发 `Port Cannot Get Tp By Tpn!`。
- [事实] 根因不是 remote link 发送本身，而是 **`CreateTp(...)` 以前只登记 connection 记录，并没有真正预创建 TP 实例**。
- [事实] 在 MPI-only 模式里，现有 `DistributedSimulatorImpl::ScheduleWithContext(...)` 不会像 hybrid 那样把事件跨 rank 投递到远端执行，因此依赖“稍后远端补建 TP”的路径不成立。
- [事实] 当前修复是在 `CreateTp(...)` 解析 `transport_channel.csv` 时，按 locality 只为 **本地拥有的 endpoint** 直接预创建 TP。
- [推论] 这条修复满足当前 smoke 路线，但 **不等价于** 完成了通用的跨 rank 动态 TP 生命周期。

### 4. 单测串跑崩溃
- [事实] 完整 `unified-bus` suite 串跑时还暴露了测试隔离问题：配置 CSV 里的 nodeId 如果固定写 `0/1`，后续测试会错误引用到前面测试创建的全局节点。
- [事实] 已修复两处：
  1. `CreateNode(...)` 开始前清空 `nodeEle_map`
  2. 新增/修改的 config-based 测试改为使用 `beforeNodes` 偏移生成 nodeId

## 当前验证记录

### 单测
```bash
build/utils/ns3.44-test-runner-default --suite=unified-bus --verbose
```
- [事实] PASS

### MPI smoke
```bash
mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-mpi-config-smoke-default --test --stop-ms=200 --case-path=scratch/ub-mpi-minimal
```
- [事实] 输出：
  - `TEST : 00000 : PASSED`

## 当前实现边界

- [事实] `ub-mpi-config-smoke` 仍明确限制在 MPI-only 路线；若 `--mtp-threads > 1` 会直接失败返回。
- [事实] 这不是 bug，而是当前实现尚未覆盖 hybrid packed-systemId case。
- [建议] 后续若要继续推进“原生支持多进程”，下一阶段应优先补：
  1. 跨 rank 动态 TP 创建控制通路
  2. control frame / CBFC 相关跨 rank 场景
  3. hybrid (`MTP + MPI`) 专用 case 与 smoke

## 建议的下一阶段

1. 做一个 **TP create control path**：
   - 源端只发轻量控制消息
   - 目标 rank 收到后转成本地 event 创建 TP
2. 基于当前 `scratch/ub-mpi-minimal` 扩成 2~3 个 topology case：
   - 单链路 device-device
   - device-switch-device
   - 开启 CBFC 的 control frame case
3. 再补一条 hybrid smoke：
   - 使用 packed systemId
   - 独立 case，不和 MPI-only case 混用
