// SPDX-License-Identifier: GPL-2.0-only
#include "ns3/command-line.h"
#include "ns3/core-module.h"
#include "ns3/ipv4-header.h"
#include "ns3/ub-header.h"
#include "ns3/mpi-interface.h"
#include "ns3/mtp-interface.h"
#include "ns3/udp-header.h"
#include "ns3/ub-app.h"
#include "ns3/ub-controller.h"
#include "ns3/ub-port.h"
#include "ns3/ub-traffic-gen.h"
#include "ns3/ub-tp-connection-manager.h"
#include "ns3/ub-utils.h"

#include <mpi.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace ns3;
using namespace utils;

namespace
{

std::vector<uint32_t> g_initialSystemIds;
bool g_packedSystemIdCheckPassed = true;
uint8_t g_expectedCbfcVl = UB_PRIORITY_DEFAULT;

struct BoundaryFrameInfo
{
    bool isControl = false;
    uint8_t firstCreditVl = 0xff;
    uint8_t firstCreditValue = 0;
};

std::vector<BoundaryFrameInfo> g_boundaryFrames;
std::vector<uint32_t> g_boundaryTpPacketSizes;

constexpr uint32_t kCbfcCellBytes = 20 * 4;
constexpr uint32_t kCbfcReturnCellGrain = 2;

void
ConfigureCase(const std::string& casePath)
{
    UbUtils::Get()->SetComponentsAttribute(casePath + "/network_attribute.txt");
    UbUtils::Get()->CreateNode(casePath + "/node.csv");
    UbUtils::Get()->CreateTopo(casePath + "/topology.csv");
    UbUtils::Get()->AddRoutingTable(casePath + "/routing_table.csv");
    UbUtils::Get()->CreateTp(casePath + "/transport_channel.csv");
}

void
LoadExpectedCbfcVl(const std::string& casePath)
{
    g_expectedCbfcVl = UB_PRIORITY_DEFAULT;
    std::ifstream file(casePath + "/transport_channel.csv");
    if (!file.is_open())
    {
        return;
    }

    std::string line;
    std::getline(file, line);
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#' || line.find_first_not_of(" \t") == std::string::npos)
        {
            continue;
        }

        std::stringstream ss(line);
        std::string item;
        for (uint32_t column = 0; column < 6; ++column)
        {
            std::getline(ss, item, ',');
        }
        if (std::getline(ss, item, ','))
        {
            g_expectedCbfcVl = static_cast<uint8_t>(std::stoul(item));
        }
        return;
    }
}

uint32_t
CountExpectedCbfcControlFrames(const std::vector<uint32_t>& packetSizes)
{
    uint32_t carriedCells = 0;
    uint32_t frameCount = 0;

    for (uint32_t packetSize : packetSizes)
    {
        const uint32_t packetCells = (packetSize + kCbfcCellBytes - 1) / kCbfcCellBytes;
        carriedCells += packetCells;
        if (carriedCells >= kCbfcReturnCellGrain)
        {
            ++frameCount;
            carriedCells %= kCbfcReturnCellGrain;
        }
    }

    return frameCount;
}

uint32_t
InitiateLocalTasks(const std::string& casePath, uint32_t mpiRank)
{
    uint32_t localTaskCount = 0;
    auto trafficData = UbUtils::Get()->ReadTrafficCSV(casePath + "/traffic.csv");
    for (const auto& record : trafficData)
    {
        Ptr<Node> sourceNode = NodeList::GetNode(record.sourceNode);
        if (UbUtils::ExtractMpiRank(sourceNode->GetSystemId()) != mpiRank)
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
CaptureInitialSystemIds()
{
    g_initialSystemIds.clear();
    g_initialSystemIds.reserve(NodeList::GetNNodes());
    for (auto nodeIt = NodeList::Begin(); nodeIt != NodeList::End(); ++nodeIt)
    {
        g_initialSystemIds.push_back((*nodeIt)->GetSystemId());
    }
}

bool
VerifyTpOwnershipPreload()
{
    for (auto nodeIt = NodeList::Begin(); nodeIt != NodeList::End(); ++nodeIt)
    {
        Ptr<Node> node = *nodeIt;
        Ptr<UbController> controller = node->GetObject<UbController>();
        if (controller == nullptr)
        {
            continue;
        }

        const bool nodeOwnedByCurrentRank = UbUtils::IsNodeOwnedByCurrentRank(node);
        const auto connections = controller->GetTpConnManager()->GetNodeConnections(node->GetId());
        for (const auto& connection : connections)
        {
            const uint32_t localTpn =
                (connection.node1 == node->GetId()) ? connection.tpn1 : connection.tpn2;
            const bool tpExists = controller->IsTPExists(localTpn);
            if (tpExists != nodeOwnedByCurrentRank)
            {
                std::cerr << "TP ownership mismatch on node " << node->GetId() << " tpn "
                          << localTpn << " ownedByRank=" << nodeOwnedByCurrentRank
                          << " exists=" << tpExists << std::endl;
                return false;
            }
        }
    }

    return true;
}

void
VerifyPackedSystemIds(uint32_t mpiRank)
{
    g_packedSystemIdCheckPassed = true;
    if (g_initialSystemIds.size() != NodeList::GetNNodes())
    {
        std::cerr << "Packed systemId verification lost initial node ownership snapshot" << std::endl;
        g_packedSystemIdCheckPassed = false;
        return;
    }

    for (uint32_t nodeId = 0; nodeId < NodeList::GetNNodes(); ++nodeId)
    {
        Ptr<Node> node = NodeList::GetNode(nodeId);
        const uint32_t initialSystemId = g_initialSystemIds[nodeId];
        const uint32_t currentSystemId = node->GetSystemId();
        const bool initiallyOwnedByCurrentRank = (initialSystemId == mpiRank);
        const bool currentlyOwnedByCurrentRank = UbUtils::IsNodeOwnedByCurrentRank(node);

        if (initiallyOwnedByCurrentRank)
        {
            if (!currentlyOwnedByCurrentRank || currentSystemId == initialSystemId ||
                (currentSystemId >> 16) == 0 || UbUtils::ExtractMpiRank(currentSystemId) != mpiRank)
            {
                std::cerr << "Packed systemId verification failed for local node " << nodeId
                          << " initial=" << initialSystemId << " current=" << currentSystemId
                          << std::endl;
                g_packedSystemIdCheckPassed = false;
                return;
            }
        }
        else
        {
            if (currentlyOwnedByCurrentRank || currentSystemId != initialSystemId ||
                UbUtils::ExtractMpiRank(currentSystemId) != initialSystemId)
            {
                std::cerr << "Packed systemId verification failed for remote node " << nodeId
                          << " initial=" << initialSystemId << " current=" << currentSystemId
                          << std::endl;
                g_packedSystemIdCheckPassed = false;
                return;
            }
        }
    }
}

void
ObserveBoundaryPortTransmit(Ptr<Packet> packet, Time)
{
    BoundaryFrameInfo info;
    Ptr<Packet> copy = packet->Copy();

    UbDatalinkHeader datalinkHeader;
    if (copy->PeekHeader(datalinkHeader) == 0)
    {
        return;
    }

    if (!datalinkHeader.IsControlCreditHeader())
    {
        UbDatalinkPacketHeader packetHeader;
        UbNetworkHeader networkHeader;
        Ipv4Header ipv4Header;
        UdpHeader udpHeader;
        UbTransportHeader transportHeader;
        if (copy->RemoveHeader(packetHeader) == 0 || copy->RemoveHeader(networkHeader) == 0 ||
            copy->RemoveHeader(ipv4Header) == 0 || copy->RemoveHeader(udpHeader) == 0 ||
            copy->RemoveHeader(transportHeader) == 0)
        {
            return;
        }

        const uint8_t tpOpcode = transportHeader.GetTPOpcode();
        if (tpOpcode == static_cast<uint8_t>(TpOpcode::TP_OPCODE_ACK_WITH_CETPH) ||
            tpOpcode == static_cast<uint8_t>(TpOpcode::TP_OPCODE_ACK_WITHOUT_CETPH))
        {
            UbCongestionExtTph congestionHeader;
            UbAckTransactionHeader ackHeader;
            if (copy->RemoveHeader(congestionHeader) == 0 || copy->RemoveHeader(ackHeader) == 0)
            {
                return;
            }
        }
        else
        {
            UbTransactionHeader transactionHeader;
            UbMAExtTah extHeader;
            if (copy->RemoveHeader(transactionHeader) == 0 || copy->RemoveHeader(extHeader) == 0)
            {
                return;
            }
        }

        g_boundaryTpPacketSizes.push_back(packet->GetSize());
        return;
    }

    UbDatalinkControlCreditHeader controlHeader;
    if (copy->RemoveHeader(controlHeader) == 0)
    {
        return;
    }

    uint8_t credits[UB_PRIORITY_NUM_DEFAULT] = {0};
    controlHeader.GetAllCreditsVL(credits);
    info.isControl = true;
    for (uint8_t vl = 0; vl < UB_PRIORITY_NUM_DEFAULT; ++vl)
    {
        if (credits[vl] > 0)
        {
            info.firstCreditVl = vl;
            info.firstCreditValue = credits[vl];
            break;
        }
    }
    g_boundaryFrames.push_back(info);
}

void
AttachBoundaryPortObservers()
{
    g_boundaryFrames.clear();
    g_boundaryTpPacketSizes.clear();
    for (auto nodeIt = NodeList::Begin(); nodeIt != NodeList::End(); ++nodeIt)
    {
        Ptr<Node> node = *nodeIt;
        for (uint32_t deviceIndex = 0; deviceIndex < node->GetNDevices(); ++deviceIndex)
        {
            Ptr<UbPort> port = DynamicCast<UbPort>(node->GetDevice(deviceIndex));
            if (port == nullptr || !port->HasMpiReceive())
            {
                continue;
            }
            port->TraceConnectWithoutContext("TraComEventNotify",
                                             MakeCallback(&ObserveBoundaryPortTransmit));
        }
    }
}

bool
ValidateObservedCbfcControlFrames(bool verifyCbfcControlCount)
{
    uint32_t controlFrameCount = 0;
    for (const auto& frame : g_boundaryFrames)
    {
        if (!frame.isControl)
        {
            continue;
        }

        ++controlFrameCount;
        if (frame.firstCreditValue == 0)
        {
            std::cerr << "Observed CBFC control frame without returned credit" << std::endl;
            return false;
        }
        if (frame.firstCreditVl != g_expectedCbfcVl)
        {
            std::cerr << "Observed CBFC control frame on VL "
                      << static_cast<uint32_t>(frame.firstCreditVl) << ", expected "
                      << static_cast<uint32_t>(g_expectedCbfcVl) << std::endl;
            return false;
        }
    }

    if (controlFrameCount == 0)
    {
        std::cerr << "No CBFC control frame observed on boundary MPI ports" << std::endl;
        return false;
    }

    if (verifyCbfcControlCount)
    {
        if (g_boundaryTpPacketSizes.empty())
        {
            std::cerr << "No boundary TP packets observed for CBFC control count verification" << std::endl;
            return false;
        }

        const uint32_t expectedCount = CountExpectedCbfcControlFrames(g_boundaryTpPacketSizes);
        if (controlFrameCount != expectedCount)
        {
            std::cerr << "Observed " << controlFrameCount
                      << " CBFC control frames on boundary MPI ports, expected " << expectedCount
                      << " from " << g_boundaryTpPacketSizes.size() << " boundary TP packets" << std::endl;
            return false;
        }
    }

    return true;
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
    bool verifyTpOwnership = false;
    bool verifyPackedSystemId = false;
    bool verifyCbfcControl = false;
    bool verifyCbfcControlCount = false;
    std::string casePath = "scratch/ub-mpi-minimal";

    CommandLine cmd(__FILE__);
    cmd.AddValue("test", "Enable regression-test style output", test);
    cmd.AddValue("stop-ms", "Simulation stop time in milliseconds", stopMs);
    cmd.AddValue("mtp-threads", "Number of local MTP threads (0-1 disables hybrid mode)", mtpThreads);
    cmd.AddValue("verify-tp-ownership",
                 "Assert config-preloaded TP instances exist only on locally owned nodes",
                 verifyTpOwnership);
    cmd.AddValue("verify-packed-systemid",
                 "Assert local nodes become packed MTP+MPI systemIds after hybrid partition",
                 verifyPackedSystemId);
    cmd.AddValue("verify-cbfc-control",
                 "Assert boundary MPI ports transmit CBFC control frames in hybrid mode",
                 verifyCbfcControl);
    cmd.AddValue("verify-cbfc-control-count",
                 "Assert CBFC control frame count matches locally received TP packets",
                 verifyCbfcControlCount);
    cmd.AddValue("case-path", "Path to the unified-bus MPI smoke case", casePath);
    cmd.Parse(argc, argv);

    if ((verifyPackedSystemId || verifyCbfcControl || verifyCbfcControlCount) && mtpThreads <= 1)
    {
        if (test)
        {
            PrintTestResult(false);
        }
        std::cerr << "hybrid verification flags require --mtp-threads > 1" << std::endl;
        return 1;
    }

    if (mtpThreads > 1)
    {
        MtpInterface::Enable(mtpThreads);
    }
    else
    {
        GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::DistributedSimulatorImpl"));
    }

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
    LoadExpectedCbfcVl(casePath);
    CaptureInitialSystemIds();
    const bool ownershipPassed = !verifyTpOwnership || VerifyTpOwnershipPreload();
    if (verifyCbfcControl)
    {
        AttachBoundaryPortObservers();
    }
    if (verifyPackedSystemId)
    {
        Simulator::ScheduleNow(&VerifyPackedSystemIds, mpiRank);
    }
    const uint32_t localTaskCount = InitiateLocalTasks(casePath, mpiRank);

    if (ownershipPassed)
    {
        Simulator::Stop(MilliSeconds(stopMs));
        Simulator::Run();
    }

    const bool cbfcControlPassed =
        !verifyCbfcControl || ValidateObservedCbfcControlFrames(verifyCbfcControlCount);

    const bool localPassed =
        ownershipPassed && g_packedSystemIdCheckPassed && cbfcControlPassed &&
        ((localTaskCount == 0) || UbTrafficGen::Get()->IsCompleted());

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
