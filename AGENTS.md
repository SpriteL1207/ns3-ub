# Repository Notes

- 对 `UbLink` / `UbRemoteLink` 的测试，默认把它们视为“搬移序列化后的 `Packet` bytes”的承载层；不要在 link 测试里引入高层协议语义假设。
- `UB_CONTROL_FRAME`、`VL`、`TP opcode`、credit 恢复等语义，应以 `UbSwitch`、flow-control、transport 等模块的实际解析与处理路径为准；优先基于这些模块已有逻辑或更高层 trace 做验证。
- 如果测试必须解析 `Packet`，应尽量复用 unified-bus 模块现有的报文解析逻辑与类型边界，避免测试 oracle 自行发明额外语义。
- 当测试 oracle 与模块真实语义冲突时，优先删除或重做该 oracle，不要为了保住测试去扭曲实现。
