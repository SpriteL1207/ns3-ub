// SPDX-License-Identifier: GPL-2.0-only
#include "ns3/command-line.h"
#include "ns3/core-module.h"
#include "ns3/mpi-interface.h"
#include "ns3/ub-app.h"
#include "ns3/ub-traffic-gen.h"
#include "ns3/ub-utils.h"

#include <mpi.h>
#include <iostream>
#include <string>

using namespace ns3;
using namespace utils;

namespace
{

uint32_t
ExtractMpiRank(uint32_t systemId)
{
#ifdef NS3_MTP
    return systemId & 0xFFFF;
#else
    return systemId;
#endif
}

void
ConfigureCase(const std::string& casePath)
{
    UbUtils::Get()->SetComponentsAttribute(casePath + "/network_attribute.txt");
    UbUtils::Get()->CreateNode(casePath + "/node.csv");
    UbUtils::Get()->CreateTopo(casePath + "/topology.csv");
    UbUtils::Get()->AddRoutingTable(casePath + "/routing_table.csv");
    UbUtils::Get()->CreateTp(casePath + "/transport_channel.csv");
}

uint32_t
InitiateLocalTasks(const std::string& casePath, uint32_t mpiRank)
{
    uint32_t localTaskCount = 0;
    auto trafficData = UbUtils::Get()->ReadTrafficCSV(casePath + "/traffic.csv");
    for (const auto& record : trafficData)
    {
        Ptr<Node> sourceNode = NodeList::GetNode(record.sourceNode);
        if (ExtractMpiRank(sourceNode->GetSystemId()) != mpiRank)
        {
            continue;
        }

        if (sourceNode->GetNApplications() == 0)
        {
            Ptr<UbApp> client = CreateObject<UbApp>();
            sourceNode->AddApplication(client);
        }

        UbTrafficGen::Get()->AddTask(record);
        localTaskCount++;
    }

    UbTrafficGen::Get()->ScheduleNextTasks();
    return localTaskCount;
}

void
PrintTestResult(bool passed)
{
    std::cout << "TEST : 00000 : " << (passed ? "PASSED" : "FAILED") << std::endl;
}

} // namespace

int
main(int argc, char* argv[])
{
#ifndef NS3_MPI
    std::cerr << "MPI support is required for ub-mpi-config-smoke" << std::endl;
    return 1;
#else
    bool test = false;
    uint32_t stopMs = 10;
    uint32_t mtpThreads = 0;
    std::string casePath = "scratch/ub-mpi-minimal";

    CommandLine cmd(__FILE__);
    cmd.AddValue("test", "Enable regression-test style output", test);
    cmd.AddValue("stop-ms", "Simulation stop time in milliseconds", stopMs);
    cmd.AddValue("mtp-threads", "Number of local MTP threads (0-1 disables hybrid mode)", mtpThreads);
    cmd.AddValue("case-path", "Path to the unified-bus MPI smoke case", casePath);
    cmd.Parse(argc, argv);

    if (mtpThreads > 1)
    {
        if (test)
        {
            PrintTestResult(false);
        }
        std::cerr << "ub-mpi-config-smoke currently validates MPI-only smoke; hybrid MTP+MPI smoke is pending a dedicated packed-systemId case" << std::endl;
        return 1;
    }

    GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::DistributedSimulatorImpl"));

    MpiInterface::Enable(&argc, &argv);

    const uint32_t mpiRank = MpiInterface::GetSystemId();
    const uint32_t mpiSize = MpiInterface::GetSize();
    if (mpiSize != 2)
    {
        if (test && mpiRank == 0)
        {
            PrintTestResult(false);
        }
        MpiInterface::Disable();
        return 1;
    }

    Time::SetResolution(Time::PS);

    ConfigureCase(casePath);
    const uint32_t localTaskCount = InitiateLocalTasks(casePath, mpiRank);

    Simulator::Stop(MilliSeconds(stopMs));
    Simulator::Run();

    const bool localPassed = (localTaskCount == 0) || UbTrafficGen::Get()->IsCompleted();

    int localPassedInt = localPassed ? 1 : 0;
    int globalPassedInt = 0;
    MPI_Allreduce(&localPassedInt,
                  &globalPassedInt,
                  1,
                  MPI_INT,
                  MPI_MIN,
                  MpiInterface::GetCommunicator());

    const bool globalPassed = (globalPassedInt != 0);

    Simulator::Destroy();
    UbUtils::Get()->Destroy();
    MpiInterface::Disable();

    if (test && mpiRank == 0)
    {
        PrintTestResult(globalPassed);
    }

    return globalPassed ? 0 : 1;
#endif
}
