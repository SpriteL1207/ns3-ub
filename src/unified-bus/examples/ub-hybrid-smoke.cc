#include "ns3/command-line.h"
#include "ns3/core-module.h"
#include "ns3/ipv4-header.h"
#include "ns3/mpi-interface.h"
#include "ns3/mtp-interface.h"
#include "ns3/network-module.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/ub-congestion-control.h"
#include "ns3/ub-controller.h"
#include "ns3/ub-datatype.h"
#include "ns3/ub-function.h"
#include "ns3/ub-header.h"
#include "ns3/ub-link.h"
#include "ns3/ub-network-address.h"
#include "ns3/ub-port.h"
#include "ns3/ub-remote-link.h"
#include "ns3/ub-switch.h"
#include "ns3/ub-tp-connection-manager.h"
#include "ns3/ub-utils.h"
#include "ns3/udp-header.h"

using namespace ns3;
using namespace utils;

namespace {
constexpr uint32_t kDataPacketSize = 64;
constexpr uint32_t kAckPacketSize = 8;
constexpr uint32_t kJettyNum = 0;
constexpr uint32_t kTaskId = 1;
constexpr uint32_t kSenderTpn = 100;
constexpr uint32_t kReceiverTpn = 200;
constexpr uint32_t kCbfcCellBytes = 20 * 4;
constexpr uint32_t kCbfcReturnCellGrain = 2;
constexpr auto kTpPriority = UB_PRIORITY_DEFAULT;

enum class FlowControlMode
{
    NONE,
    CBFC,
};

enum class RemoteFrameKind
{
    INVALID,
    DATA,
    ACK,
    CONTROL,
};

struct HybridTpTopology
{
    Ptr<Node> device0;
    Ptr<Node> switch0;
    Ptr<Node> switch1;
    Ptr<Node> device1;
    Ptr<UbPort> device0Port;
    Ptr<UbPort> switch0DevicePort;
    Ptr<UbPort> switch0RemotePort;
    Ptr<UbPort> switch1RemotePort;
    Ptr<UbPort> switch1DevicePort;
    Ptr<UbPort> device1Port;
    Ptr<UbRemoteLink> remoteLink;
};

struct RemoteFrameInfo
{
    RemoteFrameKind kind = RemoteFrameKind::INVALID;
    uint32_t psn = 0;
    uint32_t srcTpn = 0;
    uint32_t dstTpn = 0;
    uint8_t firstCreditVl = 0xff;
    uint8_t firstCreditValue = 0;
};

bool g_test = false;
bool g_senderObservedAck = false;
bool g_receiverObservedData = false;
bool g_senderTaskComplete = false;
std::vector<RemoteFrameInfo> g_remoteTpPackets;
std::vector<RemoteFrameInfo> g_remoteControlFrames;
std::vector<uint32_t> g_senderAckPacketSizes;
std::vector<uint32_t> g_receiverDataPacketSizes;
bool g_enableCbfc = false;

void
EnqueueRawPacket(Ptr<UbPort> port, uint32_t size)
{
    bool enqueued = port->GetUbQueue()->DoEnqueue(std::make_tuple(0u, 0u, Create<Packet>(size)));
    NS_ABORT_MSG_IF(!enqueued, "Failed to enqueue raw smoke packet");
    port->NotifyAllocationFinish();
}

void
TestLog(const std::string& message)
{
    if (g_test)
    {
        std::cout << "TEST " << message << std::endl;
    }
}

FlowControlMode
ParseFlowControlMode(const std::string& mode)
{
    if (mode == "none")
    {
        return FlowControlMode::NONE;
    }
    if (mode == "cbfc")
    {
        return FlowControlMode::CBFC;
    }
    NS_ABORT_MSG("Unsupported flow-control mode: " << mode);
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

RemoteFrameInfo
InspectRemoteFrame(Ptr<const Packet> packet)
{
    RemoteFrameInfo info;
    Ptr<Packet> copy = packet->Copy();

    UbDatalinkHeader datalinkHeader;
    if (copy->PeekHeader(datalinkHeader) == 0)
    {
        return info;
    }

    if (datalinkHeader.IsControlCreditHeader())
    {
        UbDatalinkControlCreditHeader controlHeader;
        if (copy->RemoveHeader(controlHeader) == 0)
        {
            return info;
        }

        uint8_t credits[UB_PRIORITY_NUM_DEFAULT] = {0};
        controlHeader.GetAllCreditsVL(credits);
        info.kind = RemoteFrameKind::CONTROL;
        for (uint8_t vl = 0; vl < UB_PRIORITY_NUM_DEFAULT; ++vl)
        {
            if (credits[vl] > 0)
            {
                info.firstCreditVl = vl;
                info.firstCreditValue = credits[vl];
                break;
            }
        }
        return info;
    }

    UbDatalinkPacketHeader dlHeader;
    if (copy->RemoveHeader(dlHeader) == 0)
    {
        return info;
    }
    if (dlHeader.GetConfig() != static_cast<uint8_t>(UbDatalinkHeaderConfig::PACKET_IPV4))
    {
        return info;
    }

    UbNetworkHeader networkHeader;
    Ipv4Header ipv4Header;
    UdpHeader udpHeader;
    UbTransportHeader transportHeader;
    if (copy->RemoveHeader(networkHeader) == 0 || copy->RemoveHeader(ipv4Header) == 0 ||
        copy->RemoveHeader(udpHeader) == 0 || copy->PeekHeader(transportHeader) == 0)
    {
        return info;
    }

    info.psn = transportHeader.GetPsn();
    info.srcTpn = transportHeader.GetSrcTpn();
    info.dstTpn = transportHeader.GetDestTpn();
    info.kind = transportHeader.GetTPOpcode() == static_cast<uint8_t>(TpOpcode::TP_OPCODE_ACK_WITH_CETPH) ||
                        transportHeader.GetTPOpcode() == static_cast<uint8_t>(TpOpcode::TP_OPCODE_ACK_WITHOUT_CETPH)
                    ? RemoteFrameKind::ACK
                    : RemoteFrameKind::DATA;
    return info;
}

void
ObserveSenderTpRecv(uint32_t,
                    uint32_t,
                    uint32_t,
                    uint32_t,
                    uint32_t srcTpn,
                    uint32_t dstTpn,
                    PacketType type,
                    uint32_t size,
                    uint32_t,
                    UbPacketTraceTag)
{
    if (type != PacketType::ACK || srcTpn != kReceiverTpn || dstTpn != kSenderTpn)
    {
        return;
    }

    g_senderAckPacketSizes.push_back(size);
    if (!g_senderObservedAck)
    {
        g_senderObservedAck = true;
        TestLog("Sender observed ack");
    }
}

void
ObserveReceiverTpRecv(uint32_t,
                      uint32_t,
                      uint32_t,
                      uint32_t,
                      uint32_t srcTpn,
                      uint32_t dstTpn,
                      PacketType type,
                      uint32_t size,
                      uint32_t,
                      UbPacketTraceTag)
{
    if (type != PacketType::PACKET || srcTpn != kSenderTpn || dstTpn != kReceiverTpn)
    {
        return;
    }

    g_receiverDataPacketSizes.push_back(size);
    if (!g_receiverObservedData)
    {
        g_receiverObservedData = true;
        TestLog("Receiver observed data");
    }
}

void
ObserveRemoteLinkTransmit(Ptr<const Packet> packet, Ptr<UbPort>, Ptr<UbPort>)
{
    RemoteFrameInfo info = InspectRemoteFrame(packet);
    if (info.kind == RemoteFrameKind::DATA || info.kind == RemoteFrameKind::ACK)
    {
        g_remoteTpPackets.push_back(info);
        return;
    }
    if (info.kind == RemoteFrameKind::CONTROL)
    {
        g_remoteControlFrames.push_back(info);
    }
}

bool
ValidateRemoteTpObservations(uint32_t systemId, uint32_t flowSize)
{
    const uint32_t expectedCount = (flowSize + UB_MTU_BYTE - 1) / UB_MTU_BYTE;
    const RemoteFrameKind expectedKind = systemId == 1 ? RemoteFrameKind::ACK : RemoteFrameKind::DATA;
    const uint32_t expectedSrcTpn = systemId == 1 ? kReceiverTpn : kSenderTpn;
    const uint32_t expectedDstTpn = systemId == 1 ? kSenderTpn : kReceiverTpn;

    if (g_remoteTpPackets.size() != expectedCount)
    {
        std::cerr << "Rank " << systemId << " observed " << g_remoteTpPackets.size()
                  << " remote TP packets, expected " << expectedCount << std::endl;
        return false;
    }

    for (uint32_t index = 0; index < expectedCount; ++index)
    {
        const RemoteFrameInfo& info = g_remoteTpPackets[index];
        if (info.kind != expectedKind)
        {
            std::cerr << "Rank " << systemId << " observed unexpected remote TP packet type at index "
                      << index << std::endl;
            return false;
        }
        if (info.psn != index)
        {
            std::cerr << "Rank " << systemId << " observed PSN " << info.psn << " at remote index "
                      << index << std::endl;
            return false;
        }
        if (info.srcTpn != expectedSrcTpn || info.dstTpn != expectedDstTpn)
        {
            std::cerr << "Rank " << systemId << " observed remote TPN tuple src=" << info.srcTpn
                      << " dst=" << info.dstTpn << ", expected src=" << expectedSrcTpn
                      << " dst=" << expectedDstTpn << std::endl;
            return false;
        }
    }

    const std::string packetType = expectedKind == RemoteFrameKind::ACK ? "ack" : "data";
    TestLog(std::string("RemoteLink observed ") + packetType + " count=" +
            std::to_string(g_remoteTpPackets.size()) + " firstPsn=" +
            std::to_string(g_remoteTpPackets.front().psn) + " lastPsn=" +
            std::to_string(g_remoteTpPackets.back().psn));
    return true;
}

bool
ValidateRemoteControlObservations(uint32_t systemId, uint32_t flowSize, FlowControlMode flowControlMode)
{
    if (flowControlMode != FlowControlMode::CBFC)
    {
        return true;
    }

    const std::vector<uint32_t>& packetSizes = systemId == 0 ? g_senderAckPacketSizes : g_receiverDataPacketSizes;
    const uint32_t expectedCount = CountExpectedCbfcControlFrames(packetSizes);
    if (packetSizes.empty())
    {
        std::cerr << "Rank " << systemId << " did not capture local TP packets for CBFC expectation" << std::endl;
        return false;
    }
    if (g_remoteControlFrames.size() != expectedCount)
    {
        std::cerr << "Rank " << systemId << " observed " << g_remoteControlFrames.size()
                  << " remote control frames, expected " << expectedCount
                  << " from " << packetSizes.size() << " local packets" << std::endl;
        return false;
    }

    for (uint32_t index = 0; index < expectedCount; ++index)
    {
        const RemoteFrameInfo& info = g_remoteControlFrames[index];
        if (info.kind != RemoteFrameKind::CONTROL)
        {
            std::cerr << "Rank " << systemId << " observed non-control frame in remote control sequence at index "
                      << index << std::endl;
            return false;
        }
        if (info.firstCreditValue == 0)
        {
            std::cerr << "Rank " << systemId << " observed remote control frame without returned credit at index "
                      << index << std::endl;
            return false;
        }
        if (info.firstCreditVl != kTpPriority)
        {
            std::cerr << "Rank " << systemId << " observed remote control frame on VL "
                      << static_cast<uint32_t>(info.firstCreditVl) << ", expected VL "
                      << static_cast<uint32_t>(kTpPriority) << std::endl;
            return false;
        }
    }

    if (g_remoteControlFrames.empty())
    {
        TestLog(std::string("RemoteLink observed control count=0 expected=") +
                std::to_string(expectedCount));
        return true;
    }

    TestLog(std::string("RemoteLink observed control count=") +
            std::to_string(g_remoteControlFrames.size()) + " expected=" +
            std::to_string(expectedCount) + " firstCreditVl=" +
            std::to_string(g_remoteControlFrames.front().firstCreditVl) + " firstCredit=" +
            std::to_string(g_remoteControlFrames.front().firstCreditValue));
    return true;
}

void
OnTpTaskCompleted(uint32_t taskId, uint32_t jettyNum)
{
    g_senderTaskComplete = true;
    TestLog("Task complete");
}

Ptr<UbPort>
CreatePort(Ptr<Node> node)
{
    Ptr<UbPort> port = CreateObject<UbPort>();
    port->SetAddress(Mac48Address::Allocate());
    node->AddDevice(port);
    return port;
}

void
InitNode(Ptr<Node> node, UbNodeType_t nodeType, uint32_t portCount)
{
    Ptr<UbSwitch> sw = CreateObject<UbSwitch>();
    node->AggregateObject(sw);
    sw->SetNodeType(nodeType);
    if (g_enableCbfc && nodeType == UB_SWITCH)
    {
        sw->SetAttribute("FlowControl", EnumValue(FcType::CBFC));
    }

    if (nodeType == UB_DEVICE)
    {
        Ptr<UbController> controller = CreateObject<UbController>();
        node->AggregateObject(controller);
        controller->CreateUbFunction();
        controller->CreateUbTransaction();
    }

    for (uint32_t index = 0; index < portCount; ++index)
    {
        CreatePort(node);
    }

    sw->Init();
    Ptr<UbCongestionControl> congestionCtrl = UbCongestionControl::Create(UB_SWITCH);
    congestionCtrl->SwitchInit(sw);
}

void
AddShortestRoute(Ptr<Node> node, uint32_t destNodeId, uint32_t destPortId, uint16_t outPort)
{
    std::vector<uint16_t> outPorts = {outPort};
    Ptr<UbRoutingProcess> routing = node->GetObject<UbSwitch>()->GetRoutingProcess();
    routing->AddShortestRoute(NodeIdToIp(destNodeId).Get(), outPorts);
    routing->AddShortestRoute(NodeIdToIp(destNodeId, destPortId).Get(), outPorts);
}

HybridTpTopology
BuildTpTopology()
{
    HybridTpTopology topo;
    topo.device0 = CreateObject<Node>(0);
    topo.switch0 = CreateObject<Node>(0);
    topo.switch1 = CreateObject<Node>(1);
    topo.device1 = CreateObject<Node>(1);

    InitNode(topo.device0, UB_DEVICE, 1);
    InitNode(topo.switch0, UB_SWITCH, 2);
    InitNode(topo.switch1, UB_SWITCH, 2);
    InitNode(topo.device1, UB_DEVICE, 1);

    topo.device0Port = DynamicCast<UbPort>(topo.device0->GetDevice(0));
    topo.switch0DevicePort = DynamicCast<UbPort>(topo.switch0->GetDevice(0));
    topo.switch0RemotePort = DynamicCast<UbPort>(topo.switch0->GetDevice(1));
    topo.switch1RemotePort = DynamicCast<UbPort>(topo.switch1->GetDevice(0));
    topo.switch1DevicePort = DynamicCast<UbPort>(topo.switch1->GetDevice(1));
    topo.device1Port = DynamicCast<UbPort>(topo.device1->GetDevice(0));

    Ptr<UbLink> leftLink = CreateObject<UbLink>();
    topo.device0Port->Attach(leftLink);
    topo.switch0DevicePort->Attach(leftLink);

    topo.remoteLink = CreateObject<UbRemoteLink>();
    topo.switch0RemotePort->Attach(topo.remoteLink);
    topo.switch1RemotePort->Attach(topo.remoteLink);
    topo.switch0RemotePort->EnableMpiReceive();
    topo.switch1RemotePort->EnableMpiReceive();

    Ptr<UbLink> rightLink = CreateObject<UbLink>();
    topo.switch1DevicePort->Attach(rightLink);
    topo.device1Port->Attach(rightLink);

    AddShortestRoute(topo.device0, topo.device1->GetId(), topo.device1Port->GetIfIndex(), topo.device0Port->GetIfIndex());
    AddShortestRoute(topo.switch0, topo.device1->GetId(), topo.device1Port->GetIfIndex(), topo.switch0RemotePort->GetIfIndex());
    AddShortestRoute(topo.switch1, topo.device1->GetId(), topo.device1Port->GetIfIndex(), topo.switch1DevicePort->GetIfIndex());

    AddShortestRoute(topo.device1, topo.device0->GetId(), topo.device0Port->GetIfIndex(), topo.device1Port->GetIfIndex());
    AddShortestRoute(topo.switch1, topo.device0->GetId(), topo.device0Port->GetIfIndex(), topo.switch1RemotePort->GetIfIndex());
    AddShortestRoute(topo.switch0, topo.device0->GetId(), topo.device0Port->GetIfIndex(), topo.switch0DevicePort->GetIfIndex());

    return topo;
}

void
EnqueueTpWqe(Ptr<Node> sender, uint32_t receiverId, uint32_t flowSize)
{
    Ptr<UbController> controller = sender->GetObject<UbController>();
    Ptr<UbFunction> function = controller->GetUbFunction();
    Ptr<UbWqe> wqe = function->CreateWqe(sender->GetId(), receiverId, flowSize, kTaskId);
    function->PushWqeToJetty(wqe, kJettyNum);
    TestLog("TP flow enqueued");
}

void
InstallStaticTpPair(const HybridTpTopology& topo)
{
    Ptr<UbController> senderCtrl = topo.device0->GetObject<UbController>();
    Ptr<UbController> receiverCtrl = topo.device1->GetObject<UbController>();

    if (!senderCtrl->IsTPExists(kSenderTpn))
    {
        Ptr<UbCongestionControl> senderCc = UbCongestionControl::Create(UB_DEVICE);
        senderCtrl->CreateTp(topo.device0->GetId(),
                             topo.device1->GetId(),
                             topo.device0Port->GetIfIndex(),
                             topo.device1Port->GetIfIndex(),
                             kTpPriority,
                             kSenderTpn,
                             kReceiverTpn,
                             senderCc);
    }

    if (!receiverCtrl->IsTPExists(kReceiverTpn))
    {
        Ptr<UbCongestionControl> receiverCc = UbCongestionControl::Create(UB_DEVICE);
        receiverCtrl->CreateTp(topo.device1->GetId(),
                               topo.device0->GetId(),
                               topo.device1Port->GetIfIndex(),
                               topo.device0Port->GetIfIndex(),
                               kTpPriority,
                               kReceiverTpn,
                               kSenderTpn,
                               receiverCc);
    }
}

void
PrepareTpFlow(Ptr<Node> sender, uint32_t receiverId, uint32_t flowSize)
{
    Ptr<UbController> controller = sender->GetObject<UbController>();
    Ptr<UbFunction> function = controller->GetUbFunction();
    Ptr<UbTransaction> transaction = controller->GetUbTransaction();

    function->CreateJetty(sender->GetId(), receiverId, kJettyNum);
    std::vector<uint32_t> tpns = {kSenderTpn};
    bool bindOk = transaction->JettyBindTp(sender->GetId(), receiverId, kJettyNum, false, tpns);
    NS_ABORT_MSG_IF(!bindOk, "Failed to bind Jetty to TP");

    Ptr<UbJetty> jetty = function->GetJetty(kJettyNum);
    NS_ABORT_MSG_IF(jetty == nullptr, "Failed to get sender jetty");
    jetty->SetClientCallback(MakeCallback(&OnTpTaskCompleted));

    TestLog("TP mode start");
    Simulator::Schedule(NanoSeconds(1), &EnqueueTpWqe, sender, receiverId, flowSize);
}

void
SetupTpMode(uint32_t systemId, const HybridTpTopology& topo, uint32_t flowSize)
{
    topo.remoteLink->SetTransmitObserver(MakeCallback(&ObserveRemoteLinkTransmit));
    Ptr<UbTransportChannel> senderTp = topo.device0->GetObject<UbController>()->GetTpByTpn(kSenderTpn);
    Ptr<UbTransportChannel> receiverTp = topo.device1->GetObject<UbController>()->GetTpByTpn(kReceiverTpn);
    NS_ABORT_MSG_IF(senderTp == nullptr, "Failed to get sender TP");
    NS_ABORT_MSG_IF(receiverTp == nullptr, "Failed to get receiver TP");
    senderTp->TraceConnectWithoutContext("TpRecvNotify", MakeCallback(&ObserveSenderTpRecv));
    receiverTp->TraceConnectWithoutContext("TpRecvNotify", MakeCallback(&ObserveReceiverTpRecv));

    if (systemId == 0)
    {
        Simulator::Schedule(NanoSeconds(10), &PrepareTpFlow, topo.device0, topo.device1->GetId(), flowSize);
    }
}
} // namespace

int
main(int argc, char* argv[])
{
    bool testing = false;
    uint32_t mtpThreads = 2;
    uint32_t flowSize = UB_MTU_BYTE;
    uint32_t stopMs = 20;
    std::string mode = "tp";
    std::string flowControl = "none";

    CommandLine cmd(__FILE__);
    cmd.AddValue("test", "Enable deterministic smoke output", testing);
    cmd.AddValue("mtp-threads", "Number of MTP threads to use", mtpThreads);
    cmd.AddValue("mode", "Smoke mode; only 'tp' is supported", mode);
    cmd.AddValue("flow-control", "Flow control mode: none or cbfc", flowControl);
    cmd.AddValue("flow-size", "TP flow size in bytes", flowSize);
    cmd.AddValue("stop-ms", "Simulation stop time in milliseconds", stopMs);
    cmd.Parse(argc, argv);

    g_test = testing;
    FlowControlMode flowControlMode = ParseFlowControlMode(flowControl);

    if (mtpThreads < 1)
    {
        mtpThreads = 1;
    }

    MtpInterface::Enable(mtpThreads);
    MpiInterface::Enable(&argc, &argv);

    const uint32_t systemId = MpiInterface::GetSystemId();
    const uint32_t systemCount = MpiInterface::GetSize();

    if (systemCount != 2)
    {
        std::cerr << "ub-hybrid-smoke requires exactly 2 MPI ranks" << std::endl;
        return 1;
    }
    if (mode != "tp")
    {
        if (systemId == 0)
        {
            if (testing)
            {
                TestLog("ERROR interceptor mode has been removed; use tp");
            }
            else
            {
                std::cerr << "interceptor mode has been removed; use tp" << std::endl;
            }
        }
        MpiInterface::Disable();
        return 1;
    }

    if (flowSize == 0)
    {
        std::cerr << "flow-size must be greater than 0" << std::endl;
        return 1;
    }

    Time::SetResolution(Time::NS);
    (void)UbFlowTag::GetTypeId();
    (void)UbPacketTraceTag::GetTypeId();
    (void)utils::UbUtils::Get();
    GlobalValue::Bind("UB_RECORD_PKT_TRACE", BooleanValue(true));
    if (flowControlMode == FlowControlMode::CBFC)
    {
        g_enableCbfc = true;
        TestLog("CBFC enabled");
    }
    HybridTpTopology topo = BuildTpTopology();
    InstallStaticTpPair(topo);

    TestLog(std::string("Rank ") + std::to_string(systemId) + " initialized");
    SetupTpMode(systemId, topo, flowSize);

    Simulator::Stop(MilliSeconds(stopMs));
    Simulator::Run();
    Simulator::Destroy();
    MpiInterface::Disable();

    if (!ValidateRemoteTpObservations(systemId, flowSize))
    {
        return 4;
    }
    if (!ValidateRemoteControlObservations(systemId, flowSize, flowControlMode))
    {
        return 8;
    }

    if (systemId == 0)
    {
        if (!g_senderObservedAck)
        {
            std::cerr << "Rank 0 did not observe TP ack" << std::endl;
            return 5;
        }
        if (!g_senderTaskComplete)
        {
            std::cerr << "Rank 0 did not complete TP task" << std::endl;
            return 6;
        }
        TestLog("PASS");
    }
    else if (systemId == 1)
    {
        if (!g_receiverObservedData)
        {
            std::cerr << "Rank 1 did not observe TP data" << std::endl;
            return 7;
        }
    }

    return 0;
}
