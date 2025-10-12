# ns-3-UB: Unified-Bus Network Simulation Framework

## 项目概述

`ns-3-UB` 是基于[灵衢基础规范](https://www.unifiedbus.com/zh)构建的 ns-3 仿真模块，实现了灵衢基础规范中功能层、事务层、传输层、网络层和数据链路层的协议框架与配套算法。本项目旨在为协议创新，网络架构探索，以及拥塞控制、流量控制、负载均衡、路由算法等网络算法研究提供仿真平台。

> 虽然我们力求尽可能贴近灵衢基础规范，但两者间仍存在差异。**用户应以灵衢基础规范作为权威指南。**

`ns-3-UB` 可用于研究基于 UB 协议的：
- 流量模式亲和，低成本，高可靠的创新拓扑架构。
- 集合通信算子、流量编排算法优化技术。
- 总线内存事务成网场景下，新的事务层序与可靠技术。
- 面向超节点网络的新内存语义传输控制技术。
- 创新自适应路由、负载均衡、拥塞控制和 QoS 优化算法。

> 本项目对**规范未指明**的策略/算法（如交换机建模方式、路由选择、拥塞标记、缓冲与仲裁策略等）提供可插拔的''参考实现''。这些实现**不属于** 灵衢基础规范的一部分，仅作示例/基线，可替换或关闭。
>
> 本项目**不包含**的功能包括但不限于：硬件内部细节建模、物理层、性能参数、控制面行为（如初始化行为、异常事件处理等）、内存管理、安全策略等。


本项目可支撑的**典型仿真功能**如下表所示。

<!-- > 下划线部分</u>表示目前demo版本中暂未实现，仅完成框架性功能，需要合作老师贡献，在10.20发布时完成。
> 合作老师后续还将实现协议未指明的更多创新算法实现，作为''参考实现''发布。 -->


<table>
  <thead>
    <tr>
      <th align="center">层级</th>
      <th align="center">能力分类</th>
      <th align="left">已支持功能</th>
      <th align="left">未完善 / 用户自定义功能</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td align="center" rowspan="2">功能层</td>
      <td>功能类型</td>
      <td>Ld/St 接口、URMA 接口</td>
      <td>URPC、Entity 相关高级功能</td>
    </tr>
    <tr>
      <td>Jetty</td>
      <td>Jetty、多对多 / 单对单通信模型</td>
      <td>Jetty Group、Jetty 状态机</td>
    </tr>
    <tr>
      <td align="center" rowspan="3">事务层</td>
      <td>服务模式</td>
      <td>ROI、ROL、UNO</td>
      <td>ROT</td>
    </tr>
    <tr>
      <td>事务序</td>
      <td>NO / RO / SO 等效功能实现</td>
      <td>—</td>
    </tr>
    <tr>
      <td>事务类型</td>
      <td>Write、Read、Send</td>
      <td>Atomic、Write_with_notify 等</td>
    </tr>
    <tr>
      <td align="center" rowspan="4">传输层</td>
      <td>服务模式</td>
      <td>RTP</td>
      <td>UTP、CTP</td>
    </tr>
    <tr>
      <td>可靠性</td>
      <td>PSN、超时重传、Go-Back-N</td>
      <td>选择性重传、快速重传</td>
    </tr>
    <tr>
      <td>拥塞控制</td>
      <td>RTP 拥塞控制机制、CAQM</td>
      <td>LDCP、DCQCN 算法、CTP 拥塞控制机制</td>
    </tr>
    <tr>
      <td>负载均衡</td>
      <td>RTP 负载均衡：TPG 机制、逐 TP / 逐包 负载均衡、乱序接收</td>
      <td>CTP 负载均衡</td>
    </tr>
    <tr>
      <td align="center" rowspan="5">网络层</td>
      <td>报文格式</td>
      <td>支持基于 IPv4 地址格式的包头、基于 16-bit CNA 地址格式的包头</td>
      <td>其余包头格式</td>
    </tr>
    <tr>
      <td>地址管理</td>
      <td>CNA 与 IP 地址转换、Primary / Port CNA</td>
      <td>用户可自定义的地址分配与转换策略</td>
    </tr>
    <tr>
      <td>路由查找</td>
      <td>基于目的地址 + 包头 RT 域段的基础路由策略、基于路径 Cost 的路由策略、基于 Hash 的 ECMP、基于负载均衡因子的逐流 / 逐包 Hash</td>
      <td>用户可自定义自适应路由等策略</td>
    </tr>
    <tr>
      <td>服务质量</td>
      <td>SL-VL 映射、基于 SP 的 VL 间调度</td>
      <td>用户可自定义的 VL 间调度策略（如 DWRR 等）</td>
    </tr>
    <tr>
      <td>拥塞标记</td>
      <td>基于包头 CC 域段的 CAQM 标记模式</td>
      <td>FECN / FECN_RTT 标记模式</td>
    </tr>
    <tr>
      <td align="center" rowspan="3">数据链路层</td>
      <td>报文收发</td>
      <td>报文粒度建模</td>
      <td>Cell / Flit 粒度建模</td>
    </tr>
    <tr>
      <td>虚通道</td>
      <td>点到点链路最多支持 16 个 VL、基于 SP 的 VL 间调度</td>
      <td>基于 DWRR 的 VL 间调度</td>
    </tr>
    <tr>
      <td>信用流控</td>
      <td>信用证独占模式、CBFC、适配 PFC</td>
      <td>控制面的信用证初始化行为、信用证共享模式</td>
    </tr>
  </tbody>
</table>

## 项目架构

```
├── README.md                   # 项目说明文档
├── toolchain/                  # 仿真工具链
│   ├── net_sim_builder.py      # 仿真配置：ns-3 网络图生成
│   ├── user_topo_*.py          # 仿真配置：用户拓扑定义文件
│   ├── xccl_rdma_maker.py      # 仿真配置：XCCL 集合通信流量生成工具
│   ├── cal_throughput.py       # 仿真输出：吞吐量计算工具
│   ├── parse_trace.py          # 仿真输出：跟踪文件解析器
│   └── task_statistics.py      # 仿真输出：任务统计分析
│
├── ns-allinone-3.44/           
│   └── ns-3.44/                # ns-3 代码框架
│       └── src/unified-bus/    # 基于灵渠基础规范的仿真组件
│           ├── model/          
│           │   ├── protocol/ub-* # 协议栈相关建模组件
│           │   └── ub-*        # 拓展算法及网元组件
│           │
│           ├── examples/       # 示例和测试用例
│           ├── test/           # 单元测试
│           └── doc/            # 文档
└── ...
```

## 核心组件

### 1. Unified Bus (UB) 模块

UB 模块是基于灵渠基础规范实现的仿真组件：

#### 网元建模组件
<p align="center">
<img src="ns-allinone-3.44/ns-3.44/src/unified-bus/doc/figures/arch2-light.D3-LpLKH.png" alt="UB Domain系统组成" width="70%">
<br>
<em>UB Domain 系统组成架构图。</em><em>来源：www.unifiedbus.com</em><br>
</p>

- **UB Controller** (`ub-controller.*`) - 执行UB 协议栈的关键组件，同时为用户提供接口
- **UB Switch** (`ub-switch.*`) - 用于UB 端口间数据转发
- **UB Port** (`ub-port.*`) - 端口抽象，处理数据包输入输出
- **UB Link** (`ub-link.*`) - 节点间的点到点连接

#### 协议栈组件
- **编程接口实例** (`ub-api-ldst*`, `ub-api-urma*`) - Load/Store 与 URMA 编程接口实例，对接功能层编程模型
- **UB Function** (`ub-function.*`) - 功能层协议框架实现，支持Load/Store，URMA编程模型
- **UB Transaction** (`ub-transaction.*`) - 事务层协议框架实现
- **UB Transport** (`ub-transport.*`) - 传输层协议框架实现
- **UB Network** (由`ub-routing-table.*`，`ub-congestion-ctrl.*`，`ub-switch.*`功能组成) - 网络层协议框架实现
- **UB Datalink** (`ub-datalink.*`) - 数据链路层协议框架实现


#### 网络算法组件
- **TP Connection Manager** (`ub-tp-connection-manager.h`) - TP Channel管理器，方便用户查找各节点TP Channel信息
- **Switch Allocator** (`ub-allocator.*`) - **UbSwitch**的关键组件，建模了交换机为数据包分配出端口的过程
- **Queue Manager** (`ub-buffer-manager.*`) - 缓冲区管理模块，影响负载均衡、流量控制、排队、丢包等行为
- **Routing Process** (`ub-routing-process.*`) - 路由表模块，实现了路由表的管理与查询功能
- **Congestion Control** (`ub-congestion-control.*`) - 拥塞控制算法框架模块
- **CAQM算法** (`ub-caqm.*`) - C-AQM拥塞控制算法实现
- **Flow Control** (`ub-flow-control.*`) - 流量控制框架模块
- **故障注入模块** (`ub-fault.*`) - 故障注入，帮助用户在特定流量进行过程中注入丢包率，高时延， 拥塞程度，错包，闪断，降lane等故障。

#### 数据类型和工具
- **Datatype** (`ub-datatype.*`) - UB 数据类型定义
- **Header** (`ub-header.*`) - UB 协议包头定义和解析
- **Network Address** (`ub-network-address.h`) - 网络地址相关工具函数，包含了地址转换、掩码匹配等功能

### 2. 核心仿真特性

#### 拓扑和流量支持
- **任意拓扑**：支持任意拓扑的仿真与建模，用户可基于 UB 工具集快速构建拓扑与路由表
- **任意流量**：支持任意仿真流量的配置，用户可基于 UB 工具集配合 UbClientDag 快速构建集合通信与通信算子图
- **性能监控**：全面的性能指标收集和分析

#### 协议栈建模
- **UB 协议栈**：支持从物理层到应用层的完整协议栈建模
- **内存语义**：实现基于 Load/Store 的内存语义行为建模
- **消息语义**：实现基于 URMA 的消息语义行为建模
- **原生多路径**：实现 TP/TP Group 协议机制实现原生多路径支持

#### 协议算法支持
- **流量控制**：实现基于信用的流量控制机制框架，兼容 PFC
- **拥塞控制**：实现拥塞控制算法常用的网侧标记、接收端回复、发送端响应框架，支持 C-AQM 算法
- **路由策略**：支持最短路由、绕路策略，支持包喷洒、ECMP 等负载均衡策略
- **QoS 支持**：提供端到端 QoS 支持，当前支持 SP 策略
- **交换仲裁**：模块化实现 UbSwitch 的交换仲裁机制建模，当前支持基于优先级的 SP 调度

### 3. 脚本工具集

提供完整的网络仿真工作流支持：

- **网络拓扑生成**：自动生成各种网络拓扑（Clos、2D Fullmesh 等）
- **流量模式生成**：支持 All-Reduce、All-to-All、All-to-All-V 等通信模式，支持 RHD、NHR、OneShot 多种集合通信算法
- **性能分析工具**：吞吐量计算、延迟分析、CDF 绘制
- **格式化结果输出**：自动生成流完成时间、带宽等基础结果信息表格，可选生成报文粒度网内逐跳信息。

## 快速开始

### 1. 环境要求

- **操作系统** Ubuntu 20.04
- **编译器**：gcc 11.4.0 +
- **构建工具**：CMake 3.12+
- **Python**：3.10+
- **依赖库**：
  - python 
    - matplotlib
    - pandas
- **ns3**: 3.44

### 2. 添加ub代码到ns3中
```bash
# 进入 ns-3 目录
# 拷贝ub代码
cp -r unified-bus ns3/src/
```

### 3. 编译构建

```bash
# 进入 ns-3 目录
cd ./ns3

# 配置构建环境
./ns3 configure --enable-tests --enable-examples

# 编译项目(可选)
./ns3 build
```

### 4. 运行示例

```bash
# 运行 UB 快速示例
./ns3 run src/unified-bus/examples/ub-quick-example.cc

# 查看运行结果
ls ./src/unified-bus/examples/cases/urma_clos/output
ls ./src/unified-bus/examples/cases/urma_clos/runlog

# 使用指定配置文件运行
./ns3 run src/unified-bus/examples/ub-quick-example.cc -- {casepath}
```

### 5. 使用 ub-toolchain 脚本生成自定义用例

```bash
# 进入脚本目录
cd ub-toolchain

# 生成拓扑，此处以4x4 2DFullMesh为例
python user_topo_4x4_2DFM.py

# 生成 All-to-All-V 流量
python all2allv_maker.py
```

## 配置文件说明

仿真配置通过 CSV 文件定义：

### 网络配置文件
- **network_attribute.txt** - 网络全局属性
- **node.csv** - 节点定义（交换机、端设备）
- **topology.csv** - 网络拓扑连接关系
- **router_table.csv** - 路由表配置
- **transport_channel.csv** - 传输通道配置
- **traffic.csv** - 流量配置

### 示例配置
```bash
# 查看示例配置
ls src/unified-bus/examples/case/urma_clos/
ls src/unified-bus/examples/case/mte_2dfm4_4_all2allv_cbfc/
```

## 仿真结果分析

### 输出文件结构
```
output/
├── task_statistics.csv    # 流量分析统计
└── throughput.csv         # 吞吐量统计
runlog/
├── PortTrace*.tr          # 端口吞吐统计
├── PakcetTrace*.tr        # 精简收发包统计
├── AllPacketTrace*.tr     # 所有包路径统计
└── TaskTrace*.tr          # 流量任务统计
```

### 性能指标
- **吞吐量**：链路和端到端吞吐量统计
- **延迟**：数据包传输延迟分布
- **丢包率**：网络拥塞导致的丢包统计
- **缓冲区利用率**：交换机缓冲区使用情况
- **流量分布**：网络负载均衡分析

## 扩展开发

### 添加新的网络组件

1. 在 `src/unified-bus/model/` 目录下创建新的 `.h` 和 `.cc` 文件
2. 更新 `src/unified-bus/CMakeLists.txt` 文件添加新文件到构建系统
3. 实现 ns-3 对象接口（继承 `Object` 或相关基类）
4. 添加对应的测试用例

### 自定义拓扑
```python
# 在 ub-toolchain/ 目录下创建新的拓扑文件
# 参考现有的 user_topo_*.py 实现自定义拓扑
```

### 新增流量模式
```python
# 参考 xccl_rdma_maker.py 实现新的流量生成器
# 生成符合格式的 traffic.csv 文件
```

## 调试和日志

### 启用日志输出
```cpp
// 在仿真代码中启用特定组件日志
LogComponentEnable("UbSwitch", LOG_LEVEL_INFO);
LogComponentEnable("UbPort", LOG_LEVEL_DEBUG);
LogComponentEnable("UbFlowControl", LOG_LEVEL_ALL);
LogComponentEnableAll(LOG_PREFIX_TIME);
```

### 常用调试选项
- `LOG_LEVEL_ERROR`  - 仅错误信息
- `LOG_LEVEL_WARN`   - 警告和错误
- `LOG_LEVEL_INFO`   - 一般信息
- `LOG_LEVEL_DEBUG`  - 调试信息
- `LOG_LEVEL_ALL`    - 所有日志
- `LOG_PREFIX_TIME`  - 添加仿真时间前缀

### 内存管理
- UB 模块使用 ns-3 智能指针（`Ptr<>`）进行内存管理
- 避免在仿真循环中创建大量临时对象
- 合理设置仿真停止条件

## 贡献指南

1. **代码规范**：遵循 ns-3 编码规范
2. **文档**：为新功能添加完整的文档注释
3. **测试**：确保新代码通过所有测试用例
4. **性能**：验证修改不会显著影响仿真性能

## 许可证

本项目遵循 ns-3 许可证协议，GPL v2。详见 `LICENSE` 文件。

## 技术支持

如有技术问题或功能请求，请通过项目 Issues 进行反馈。

## 版本信息

**当前版本**：v0.9
**适配平台**：ns-3.44  
**发布状态**：Demo框架

---

*文档最后更新：2025年9月*