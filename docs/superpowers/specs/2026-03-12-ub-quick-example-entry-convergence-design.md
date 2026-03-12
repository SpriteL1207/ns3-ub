# UB Quick Example Entry Convergence Design

**Context**
- `ub-quick-example` 要作为 unified-bus 的统一用户入口。
- `UbUtils` 当前不是纯 example helper，而是配置驱动场景构建与运行时接线组件。
- 本轮不引入独立 loader/parser；只做最小边界收口。

**Design**
- `src/unified-bus/examples/ub-quick-example.cc` 保留用户入口职责：CLI、case-path 校验、runtime 选择、按顺序调用 builder/wiring 原语。
- `src/unified-bus/model/ub-utils.{h,cc}` 保留 config-driven builder/runtime wiring 职责，不再被描述成“纯读取工具”。
- traffic 相关接口保留“边读边配置 `UbTrafficGen`”行为，但接口命名必须体现副作用，避免 `Read*` 误导。
- `scratch/ub-quick-example.cc` 视为旧入口示例残片；若与正式入口重复，则不继续作为本轮交付的一部分。
- 与本轮目标无关的脏改动单独回退，不混入入口收口提交。

**Success Criteria**
- `ub-quick-example` 仍是唯一 config-driven 用户入口。
- `UbUtils` 接口命名与职责一致。
- 无关 header 脏改动被清理出本轮提交。
- `unified-bus-examples` 回归通过。
