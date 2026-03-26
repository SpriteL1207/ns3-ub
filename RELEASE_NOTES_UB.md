# ns-3-UB 发布说明

**语言**: [English](RELEASE_NOTES_UB_en.md) | [中文](RELEASE_NOTES_UB.md)

## Release 1.2.0

**发布日期**: 2026 年 3 月

### 新特性

- **OpenUSim Agent Skill 体系**：新增仓库内置的四阶段 AI Agent Skill，让 AI 编码助手（Codex / Claude Code / Cursor 等）能够端到端地完成 UB 仿真实验。四个阶段分别为：环境就绪检查（welcome）、实验规划与参数收敛（plan-experiment）、case 生成与仿真执行（run-experiment）、结果解读与根因分析（analyze-results）。配套提供 AGENTS.md 路由规则和共享知识库（拓扑选项、负载模式、trace 可观测性等 7 份参考文档），以及仿真配置文件自动生成脚本

- **URMA Read 全链路支持**：实现完整的 URMA Read 请求/响应数据通路。Read 请求以零 payload 方式发送并携带逻辑字节数，对端收到后自动生成 Read Response 回传实际数据。Transport 层支持多包 Read 响应的重组与完成判定，Transaction 层区分 Request/Response 方向并正确处理 Read 与 Write 的不同完成语义

- **流控与缓冲区管理重构**：
  - **共享缓冲区动态准入控制**：重构入口缓冲区管理，采用 Reserve → Shared → Headroom 三层准入模型。每个入口队列拥有独立保留配额（Reserve），超出部分按动态阈值（Alpha）从全局共享池竞争分配，PFC 场景下进一步使用端口级 Headroom 吸收在途报文。支持 XOFF/XON 水位查询与防振荡偏移
  - **CBFC / PFC 流控模式完善**：流控模式扩展为五种——NONE、CBFC（独占 credit）、CBFC_SHARED（共享 credit 池）、PFC_FIXED（固定阈值反压）、PFC_DYNAMIC（基于缓冲区占用的动态阈值反压）。CBFC 与 PFC 作为对等的流控策略共用同一套入口缓冲区准入模型，可按场景灵活选择

- **MPI 多进程数据通路**：新增远程链路抽象，支持通过 MPI 跨进程传输 UB 报文，实现分布式多进程仿真。配合统一的 quick-example 入口，支持 MPI 配置驱动的多机拓扑仿真

### 功能优化

- **仿真进度停滞告警**：case-runner 实时监控任务完成进度，当长时间无任务完成时输出潜在死锁警告，便于快速定位流控死锁或路由环路等问题
- **细粒度 Tracing**：支持模块级 trace 开关，按需启用或禁用不同协议层的 trace 输出，降低大规模仿真的 I/O 开销
- **可观测性分级预设**：提供多级观测预设，在快速验证和深度分析场景间一键切换日志详细程度
- **TrafficGen 线程安全**：流量生成器支持 UNISON 多线程并发调度下的安全调用
- **TrafficGen 支持 URMA Read**：流量描述文件支持指定 URMA_READ 操作类型
- **统一仿真入口**：ub-quick-example 重构为配置驱动的统一入口，同时支持 MPI 多进程和 MTP 多线程两种运行模式

### Bug 修复

- 修复 TA 层调度 WQE Segment 时的公平性问题，消除部分 segment 饥饿
- 修复多包 URMA Read 请求的切片重组逻辑，保证数据完整性
- 修复路由 trace 记录的端口信息错误和 VOQ 索引越界检查
- 修复部分构建配置下 unified-bus 库链接缺失导致的编译失败
- 修复初始化阶段的竞态条件，提升启动稳定性
- MPI 相关测试按构建标志条件编译，非 MPI 构建不再报错

### 构建与 CI

- CI 流水线简化为 Ubuntu 单平台
- 新增 uv.lock 依赖锁文件，指定 Python 3.11
- 更新 ns-3-ub-tools 子模块

### 测试

- 新增 URMA Read、共享缓冲区准入、MPI CBFC 混合模式等多组回归测试
- 新增 TrafficGen 和 quick-example 入口边界测试
- 新增 Agent Skill 文档与脚本辅助函数测试
- 测试代码净增约 2800 行

---

## Release 1.1.0

**发布日期**: 2026 年 1 月

### 新特性

- **UNISON 多线程并行仿真**：集成 UNISON 框架，支持多线程并行仿真
- **DWRR 调度算法**：在网络层和数据链路层新增基于 DWRR（Deficit Weighted Round Robin）的 VL 间调度支持
- **自适应路由**：实现基于端口负载感知的自适应路由，支持可配置的路由属性
- **死锁检测**：在交换机和传输层新增潜在死锁检测，增强报文到达时间追踪
- **CBFC 共享信用模式**：引入 CBFC 共享信用模式，提供更灵活的流控配置

### 优化与 Bug 修复

- 优化 DWRR 用户配置方式
- 重构缓冲区管理架构，统一 VOQ 管理（双视图 + 出口统计）
- 优化路由表查找流程
- 改进队列管理，支持基于字节的出口队列限制
- 修复 LDST 与 CBFC 的兼容性问题
- 优化流量控制配置接口
- 修复交换仲裁器中的 TP 移除与信用恢复问题
- 支持无配置文件自动生成 TP
- 支持无用 TP 自动移除优化

---

## Release 1.0.0

ns-3-UB 仿真器初始版本，实现了灵衢基础规范中功能层、事务层、传输层、网络层和数据链路层的完整协议栈支持。

**核心特性：**
- 完整的 UB 协议栈实现
- 支持 Load/Store 和 URMA 编程接口
- 拥塞控制与流量控制机制
- 多路径路由与负载均衡
- 基于 SP 调度的 QoS 支持
- 基于信用的流量控制（CBFC）
