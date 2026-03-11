## 结论

- `ub-quick-example` 现在承接 unified-bus 配置驱动场景的统一入口。
- 旧 smoke/config-runner 路径已从源码与构建接线删除。
- `mpi-test-suite` 里的 unified-bus MPI 回归已切到 `ub-quick-example --test`。
- 白盒覆盖仍放在 `src/unified-bus/test/ub-test.cc`，黑盒 MPI 回归入口保留在 `src/mpi/test/mpi-test-suite.cc`。

## 代码改动

- `src/unified-bus/examples/ub-quick-example.cc`
  - 保留显式建模/运行步骤。
  - 增加 `--test`，用于 `ExampleAsTestCase` 风格 MPI 回归。
  - 在测试模式下输出 `TEST : 00000 : PASSED/FAILED`。
- `src/mpi/test/mpi-test-suite.cc`
  - 将 unified-bus 相关 MPI suites 切到 `src/unified-bus/examples/ub-quick-example`。
  - 保留原 suite 名，复用既有 `*.reflog`，减少回归入口变动。
  - 删除旧 smoke 专属的 TP ownership / CBFC suites。
- `src/unified-bus/examples/CMakeLists.txt`
  - 删除旧 dedicated smoke 目标及其构建接线。
  - 将保留的 TP-only MPI regression binary 重命名为 `ub-mtp-remote-tp-regression`。
- 删除文件
  - 旧 dedicated smoke/config runner 源文件
- 删除旧 reflog
  - `src/mpi/test/mpi-example-ub-mpi-config-tp-ownership-2.reflog`
  - `src/mpi/test/mpi-example-ub-mpi-config-hybrid-cbfc-2.reflog`
  - `src/mpi/test/mpi-example-ub-mpi-config-hybrid-cbfc-multivl-2.reflog`

## 验证

- 构建
  - `cmake --build /Users/ytxing/workspace/ns-3-ub-mpi/cmake-cache -j 7 --target ub-quick-example unified-bus-test mpi-test`
- 单元 / system
  - `build/utils/ns3.44-test-runner-default --suite=unified-bus --verbose`
  - `build/utils/ns3.44-test-runner-default --suite=unified-bus-examples --verbose`
- MPI black-box
  - 四条 config-driven unified-bus MPI suites
  - `build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mtp-remote-tp-regression-np2 --verbose`
  - `build/utils/ns3.44-test-runner-default --suite=mpi-example-ub-mtp-remote-tp-regression-interceptor-removed-np2 --verbose`
  - regression-only binary spot check: `mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-mtp-remote-tp-regression-default --test`

## 仍然保留的边界

- `ub-mtp-remote-tp-regression` 仍保留，作为 regression-only binary；它不是 unified-bus 用户入口。
- 本轮没有继续保留 smoke 中的 CBFC / TP ownership 专用黑盒 oracle。
  - 原因：这些检查要么已经下沉为白盒 ownership/helper 覆盖，要么属于更高层协议语义，不应继续绑在旧 smoke 壳上。
- 本轮没有为 task activation ownership 单独引入新公共 API；当前依赖现有 ownership helper 白盒测试与 quick-example 黑盒回归共同覆盖。
