# 仓库指南


## 项目结构与模块组织

```
ns-3-ub/
├── src/unified-bus/          # UB 协议栈实现（核心开发区）
│   ├── model/                # 所有 C++ 模型代码
│   │   └── protocol/         # 协议层（function/transaction/transport/routing/flow-control/caqm/header）
│   ├── test/ub-test.cc       # 单测文件
│   ├── examples/             # ub-quick-example 用户后端与相关示例
│   └── doc/                  # switch-buffer-architecture.md
├── scratch/                  # 仿真场景入口与 CSV case 数据（15 个 case 目录）
│   ├── ub-quick-example.cc   # 对 example/ub-quick-example 的薄封装
│   └── <case>/               # 各场景: *_single-tp, clos_*, 2dfm4x4_*, ...（含 CSV 配置）
├── dev-knowledge/            # 知识基线（规范文档、经验沉淀）
│   └── specification/        # UB Base Specification 2.0 PDF + key-index
├── src/mtp/                  # Unison 多线程仿真模块（非标准 ns-3）
├── utils/                    # 辅助脚本（run-examples, create-module.py 等）
├── scratch/ns-3-ub-tools/    # 子模块: 拓扑/流量生成 + trace 解析
└── build/                    # 构建产物（忽略）
```

## 代码风格与命名约定

遵循 `.editorconfig` 与 `.clang-format` (BasedOnStyle=Microsoft, C++20, ColumnLimit=100)。
- 缩进: `*.cc`/`*.h`/`*.py` 使用 4 空格；CMake/YAML/Markdown 使用 2 空格；`Makefile` 使用 tab。
- UB 文件命名: `ub-*.cc/.h`；协议层代码置于 `model/protocol/`。
- 类命名: PascalCase + Ub 前缀（`UbPort`, `UbTransaction`）。
- 场景命名: 延续 `2nodes_*`、`clos_*`、`2dfm4x4_*`。
- 指针对齐: `Type* ptr`（PointerAlignment=Left）。

## 构建与验证

### 首次配置
```bash
./ns3 configure --enable-asserts --enable-examples --enable-tests --disable-werror -d release -G Ninja
```

### 快速验证（推荐）
开发时只需运行场景验证，`./ns3 run` 会自动按需构建所需模块：
```bash
# 运行场景（自动构建依赖）
./ns3 run 'scratch/ub-quick-example --case-path=scratch/2nodes_single-tp'

# 后续运行（复用已构建产物）
./ns3 run --no-build 'scratch/ub-quick-example --case-path=scratch/2nodes_single-tp'
```

### 模块级构建（可选）
如需单独构建模块而不运行场景：
```bash
ninja -C cmake-cache unified-bus unified-bus-test
```

### 单元测试
```bash
# 构建 test-runner（首次或测试代码修改后）
ninja -C cmake-cache test-runner

# 运行指定模块的测试（test-name 为运行时过滤）
./build/utils/ns3.44-test-runner-default --test-name=unified-bus
```

### 全量构建与测试（必要时）

**构建说明**：
- `./ns3 build`（无参数）= 构建所有模块
- `./ns3 run '<program>'` = 按需构建所需目标及其依赖，然后执行
- `./ns3 run --no-build` = 不触发构建，直接运行已编译的程序

**仅以下情况需要全量构建**：
- 修改了 `src/core/` 等基础模块
- 修改了 CMake 配置或头文件接口
- 准备提交前最终验证
- CI/CD 流水线

```bash
# 全量构建所有模块
./ns3 build

# 全量测试
./test.py
```


## 提交与 PR 规范

### 提交格式
Conventional Commit: `type(scope): subject`
- `feat(scope): ...` - 新功能
- `fix(scope): ...` - 修复
- `docs(scope): ...` - 文档
- `refactor(scope): ...` - 重构
- `test(scope): ...` - 测试
- `chore: ...` - 杂项

标题要求：祈使句、现在时、≤72 字符。

### PR 内容清单
- [ ] 问题背景与改动范围
- [ ] 关键设计或行为变化
- [ ] 实际执行过的验证命令
- [ ] `Knowledge Sources` 小节（引用 dev-knowledge 路径 + 采用原因）
- [ ] 涉及流程或输出变化时附日志/截图

## ANTI-PATTERNS

- ❌ 开发时进行不必要的全量构建/测试（浪费资源，用模块级代替）
- ❌ <IMPORTANT>电脑性能有限，永远不要并行build！build任务和测试任务分离，build必须串行！</IMPORTANT>


# Repository Notes

- 对 `UbLink` / `UbRemoteLink` 的测试，默认把它们视为“搬移序列化后的 `Packet` bytes”的承载层；不要在 link 测试里引入高层协议语义假设。
- `UB_CONTROL_FRAME`、`VL`、`TP opcode`、credit 恢复等语义，应以 `UbSwitch`、flow-control、transport 等模块的实际解析与处理路径为准；优先基于这些模块已有逻辑或更高层 trace 做验证。
- 如果测试必须解析 `Packet`，应尽量复用 unified-bus 模块现有的报文解析逻辑与类型边界，避免测试 oracle 自行发明额外语义。
- 当测试 oracle 与模块真实语义冲突时，优先删除或重做该 oracle，不要为了保住测试去扭曲实现。
