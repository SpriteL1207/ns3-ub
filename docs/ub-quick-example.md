# `ub-quick-example` 使用说明

`ub-quick-example` 用来读取一个 case 目录中的配置文件，并据此创建 unified-bus 场景、运行仿真、输出 trace 与统计结果。

推荐入口：

```bash
./ns3 run 'scratch/ub-quick-example --case-path=scratch/<case-dir>'
```

如果你已经启用了 examples，也可以使用：

```bash
./ns3 run 'src/unified-bus/examples/ub-quick-example --case-path=scratch/<case-dir>'
```

## 支持的运行方式

| 运行方式 | 是否支持 | 说明 |
|---|---|---|
| 单进程单线程 | 支持 | 默认模式 |
| 单进程 MTP 多线程 | 支持 | 使用 `--mtp-threads=<N>`，`N>=2` |
| MPI 多进程 + `traffic.csv` | 不支持 | 会直接退出并打印提示 |

## 构建

默认推荐直接使用 `scratch` 入口：

```bash
./ns3 configure
./ns3 build
```

如果要启用 MTP：

```bash
./ns3 configure --enable-mtp
./ns3 build
```

如果要运行 `src/unified-bus/examples/ub-quick-example`，需要额外启用 examples：

```bash
./ns3 configure --enable-examples
./ns3 build
```

## 常用命令

单进程：

```bash
./ns3 run 'scratch/ub-quick-example --case-path=scratch/2nodes_single-tp'
```

单进程 MTP 多线程：

```bash
./ns3 run 'scratch/ub-quick-example --case-path=scratch/2nodes_single-tp --mtp-threads=2'
```

测试模式：

```bash
./ns3 run 'scratch/ub-quick-example --case-path=scratch/2nodes_single-tp --test'
```

如果你需要显式使用 example 入口：

```bash
./ns3 run 'src/unified-bus/examples/ub-quick-example --case-path=scratch/2nodes_single-tp --test'
```

## `case-path` 目录

最小 case 目录需要包含：

- `network_attribute.txt`
- `node.csv`
- `topology.csv`
- `routing_table.csv`
- `traffic.csv`

可选文件：

- `transport_channel.csv`
- `fault.csv`

更完整的字段定义、约束和示例见： `scratch/README.md`

## 程序会做什么

程序按下面的顺序工作：

1. 解析 CLI 参数
2. 校验 `--case-path`
3. 读取 `network_attribute.txt`
4. 创建 node、topology、routing、TP
5. 读取 `traffic.csv`
6. 激活应用并运行仿真
7. 解析 trace 并输出统计结果

## 常见提示

### `missing required case path (--case-path or casePath)`

没有提供 case 目录。请使用：

```bash
./ns3 run 'scratch/ub-quick-example --case-path=scratch/<case-dir>'
```

### `missing required case file`

`case-path` 目录下缺少必需文件。请检查上面的最小文件列表。

### MPI 多进程模式下直接退出

如果你看到 `UbTrafficGen does not support MPI multi-process usage in this branch`，含义是：当前版本中，`traffic.csv` 驱动的任务生成仅支持单进程运行。

因此，下面这类命令会被拒绝：

```bash
mpirun -np 2 build/scratch/ns3.44-ub-quick-example-default --case-path=scratch/ub-mpi-minimal --test
```

### trace 后处理没有输出

请检查：

- `UB_PARSE_TRACE_ENABLE`
- `UB_PYTHON_SCRIPT_PATH`

## 相关文档

- 快速开始： `QUICK_START.md`
- case 配置格式： `scratch/README.md`
- Unison / MTP 背景： `UNISON_README.md`
