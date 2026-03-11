// SPDX-License-Identifier: GPL-2.0-only
#ifndef UB_CONFIG_RUNNER_H
#define UB_CONFIG_RUNNER_H

#include <cstdint>
#include <functional>
#include <string>

namespace ns3
{

struct UbConfigRunOptions
{
    std::string casePath;
    uint32_t mtpThreads = 0;
    uint32_t stopMs = 0;
    bool enableMpi = false;
    bool activateLocalOwnedTasksOnly = false;
    int* argc = nullptr;
    char*** argv = nullptr;
};

struct UbConfigRunResult
{
    bool localPassed = true;
    bool globalPassed = true;
    uint32_t localTaskCount = 0;
    uint32_t mpiRank = 0;
    uint32_t mpiSize = 1;
    bool mpiEnabled = false;
};

struct UbConfigRunnerHooks
{
    std::function<void(const UbConfigRunOptions&, UbConfigRunResult&)> onConfigured;
    std::function<void(const UbConfigRunOptions&, UbConfigRunResult&)> onTasksActivated;
    std::function<void(const UbConfigRunOptions&, UbConfigRunResult&)> onAfterRun;
};

bool
UbDetectMpiWorld();

UbConfigRunResult
RunUbConfigCase(const UbConfigRunOptions& options, const UbConfigRunnerHooks& hooks = {});

} // namespace ns3

#endif // UB_CONFIG_RUNNER_H
