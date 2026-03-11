// SPDX-License-Identifier: GPL-2.0-only
#include "ub-config-runner.h"

#include "ns3/boolean.h"
#include "ns3/config.h"
#include "ns3/core-module.h"
#include "ns3/global-value.h"
#include "ns3/node-list.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/ub-app.h"
#include "ns3/ub-traffic-gen.h"
#include "ns3/ub-utils.h"

#ifdef NS3_MPI
#include "ns3/mpi-interface.h"
#include <mpi.h>
#endif

#ifdef NS3_MTP
#include "ns3/mtp-interface.h"
#endif

#include <cstdlib>

namespace ns3
{

namespace
{

bool
ShouldInitializeMpi(const UbConfigRunOptions& options)
{
#ifdef NS3_MPI
    return options.enableMpi;
#else
    (void)options;
    return false;
#endif
}

void
ConfigureSimulator(const UbConfigRunOptions& options)
{
#ifdef NS3_MPI
    if (options.enableMpi)
    {
#ifdef NS3_MTP
        if (options.mtpThreads > 1)
        {
            MtpInterface::Enable(options.mtpThreads);
        }
        else
#endif
        {
            GlobalValue::Bind("SimulatorImplementationType",
                              StringValue("ns3::DistributedSimulatorImpl"));
        }
        return;
    }
#endif

#ifdef NS3_MTP
    if (options.mtpThreads > 1)
    {
        Config::SetDefault("ns3::MultithreadedSimulatorImpl::MaxThreads",
                           UintegerValue(options.mtpThreads));
        GlobalValue::Bind("SimulatorImplementationType",
                          StringValue("ns3::MultithreadedSimulatorImpl"));
        return;
    }
#endif

    (void)options;
}

void
ConfigureCase(const std::string& casePath)
{
    utils::UbUtils::Get()->SetComponentsAttribute(casePath + "/network_attribute.txt");
    utils::UbUtils::Get()->CreateNode(casePath + "/node.csv");
    utils::UbUtils::Get()->CreateTopo(casePath + "/topology.csv");
    utils::UbUtils::Get()->AddRoutingTable(casePath + "/routing_table.csv");
    utils::UbUtils::Get()->CreateTp(casePath + "/transport_channel.csv");
}

uint32_t
InitiateTasks(const UbConfigRunOptions& options, const UbConfigRunResult& result)
{
    auto trafficData = utils::UbUtils::Get()->ReadTrafficCSV(options.casePath + "/traffic.csv");

    BooleanValue faultEnabled;
    utils::UbUtils::Get()->g_fault_enable.GetValue(faultEnabled);
    if (faultEnabled.Get())
    {
        utils::UbUtils::Get()->InitFaultMoudle(options.casePath + "/fault.csv");
    }

    uint32_t localTaskCount = 0;
    for (const auto& record : trafficData)
    {
        Ptr<Node> sourceNode = NodeList::GetNode(record.sourceNode);
        if (options.activateLocalOwnedTasksOnly &&
            utils::UbUtils::ExtractMpiRank(sourceNode->GetSystemId()) != result.mpiRank)
        {
            continue;
        }

        if (sourceNode->GetNApplications() == 0)
        {
            Ptr<UbApp> client = CreateObject<UbApp>();
            sourceNode->AddApplication(client);
            utils::UbUtils::Get()->ClientTraceConnect(record.sourceNode);
        }

        UbTrafficGen::Get()->AddTask(record);
        ++localTaskCount;
    }

    utils::UbUtils::Get()->PrintTimestamp("Start Client.");
    UbTrafficGen::Get()->ScheduleNextTasks();
    return localTaskCount;
}

bool
ReadPositiveEnvVar(const char* name)
{
    const char* value = std::getenv(name);
    if (value == nullptr || *value == '\0')
    {
        return false;
    }

    char* end = nullptr;
    const unsigned long parsed = std::strtoul(value, &end, 10);
    return end != value && *end == '\0' && parsed > 1;
}

} // namespace

bool
UbDetectMpiWorld()
{
#ifdef NS3_MPI
    return ReadPositiveEnvVar("OMPI_COMM_WORLD_SIZE") || ReadPositiveEnvVar("PMI_SIZE") ||
           ReadPositiveEnvVar("PMIX_SIZE") || ReadPositiveEnvVar("MV2_COMM_WORLD_SIZE");
#else
    return false;
#endif
}

UbConfigRunResult
RunUbConfigCase(const UbConfigRunOptions& options, const UbConfigRunnerHooks& hooks)
{
    UbConfigRunResult result;
    result.mpiEnabled = ShouldInitializeMpi(options);

    ConfigureSimulator(options);

#ifdef NS3_MPI
    if (result.mpiEnabled)
    {
        int localArgc = 0;
        char** localArgv = nullptr;
        int* argc = options.argc != nullptr ? options.argc : &localArgc;
        char*** argv = options.argv != nullptr ? options.argv : &localArgv;
        MpiInterface::Enable(argc, argv);
        result.mpiRank = MpiInterface::GetSystemId();
        result.mpiSize = MpiInterface::GetSize();
    }
#endif

    ConfigureCase(options.casePath);
    if (hooks.onConfigured)
    {
        hooks.onConfigured(options, result);
    }

    result.localTaskCount = InitiateTasks(options, result);
    if (hooks.onTasksActivated)
    {
        hooks.onTasksActivated(options, result);
    }

    if (options.stopMs > 0)
    {
        Simulator::Stop(MilliSeconds(options.stopMs));
    }

    Simulator::Run();

    result.localPassed = (result.localTaskCount == 0) || UbTrafficGen::Get()->IsCompleted();
    result.globalPassed = result.localPassed;

#ifdef NS3_MPI
    if (result.mpiEnabled)
    {
        int localPassedInt = result.localPassed ? 1 : 0;
        int globalPassedInt = 0;
        MPI_Allreduce(&localPassedInt,
                      &globalPassedInt,
                      1,
                      MPI_INT,
                      MPI_MIN,
                      MpiInterface::GetCommunicator());
        result.globalPassed = (globalPassedInt != 0);
    }
#endif

    if (hooks.onAfterRun)
    {
        hooks.onAfterRun(options, result);
    }

    Simulator::Destroy();
    utils::UbUtils::Get()->Destroy();

#ifdef NS3_MPI
    if (result.mpiEnabled && MpiInterface::IsEnabled())
    {
        MpiInterface::Disable();
    }
#endif

    return result;
}

} // namespace ns3
