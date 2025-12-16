// SPDX-License-Identifier: GPL-2.0-only
#include "ns3/ub-switch-allocator.h"
#include "ns3/ub-switch.h"
#include "ns3/ub-port.h"
#include "protocol/ub-routing-process.h"
#include "protocol/ub-transport.h"
#include "ub-queue-manager.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(UbSwitchAllocator);
NS_LOG_COMPONENT_DEFINE("UbSwitchAllocator");

TypeId UbSwitchAllocator::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::UbSwitchAllocator")
        .SetParent<Object>()
        .AddConstructor<UbSwitchAllocator>()
        .AddAttribute("AllocationTime",
                      "Time of Allocation Used.",
                      TimeValue(NanoSeconds(10)),
                      MakeTimeAccessor(&UbSwitchAllocator::m_allocationTime),
                      MakeTimeChecker());
    return tid;
}
UbSwitchAllocator::UbSwitchAllocator()
{
}

UbSwitchAllocator::~UbSwitchAllocator()
{
}

void UbSwitchAllocator::DoDispose()
{
    m_ingressSources.clear();
}

void UbSwitchAllocator::TriggerAllocator(Ptr<UbPort> outPort)
{
}

void UbSwitchAllocator::Init()
{
    Simulator::Schedule(MilliSeconds(10), &UbSwitchAllocator::CheckDeadlock, this);
}
void UbSwitchAllocator::RegisterUbIngressQueue(Ptr<UbIngressQueue> ingressQueue, uint32_t outPort, uint32_t priority)
{
    m_ingressSources[outPort][priority].push_back(ingressQueue);
}

void UbSwitchAllocator::CheckDeadlock()
{
    Time now = Simulator::Now();
    Time threshold = MilliSeconds(10);

    for (uint32_t outPort = 0; outPort < m_ingressSources.size(); ++outPort) {
        for (uint32_t pri = 0; pri < m_ingressSources[outPort].size(); ++pri) {
            for (const auto& queue : m_ingressSources[outPort][pri]) {
                if (queue && !queue->IsEmpty()) {
                    if (now - queue->GetHeadArrivalTime() > threshold) {
                        std::stringstream ss;
                        ss << "Potential Deadlock in Node " << m_nodeId 
                           << " OutPort:" << outPort << " Pri:" << pri;
                        
                        if (queue->GetIngressQueueType() == IngressQueueType::VOQ) {
                            ss << " QueueType:VOQ InPort:" << queue->GetInPortId();
                        } else if (queue->GetIngressQueueType() == IngressQueueType::TP) {
                            auto tp = DynamicCast<UbTransportChannel>(queue);
                            if (tp) {
                                ss << " QueueType:TP TPN:" << tp->GetTpn();
                            } else {
                                ss << " QueueType:TP (Cast Failed)";
                            }
                        } else {
                            ss << " QueueType:Unknown(" << (int)queue->GetIngressQueueType() << ")";
                        }

                        ss << " Head packet stuck for " << (now - queue->GetHeadArrivalTime()).GetMilliSeconds() << " ms";
                        NS_LOG_WARN(ss.str());
                    }
                }
            }
        }
    }
    Simulator::Schedule(MilliSeconds(10), &UbSwitchAllocator::CheckDeadlock, this);
}

void UbSwitchAllocator::RegisterEgressStauts(uint32_t portsNum)
{
    m_egressStatus.resize(portsNum, true);
}

void UbSwitchAllocator::SetEgressStatus(uint32_t portId, bool status)
{
    m_egressStatus[portId] = status;
}

bool UbSwitchAllocator::GetEgressStatus(uint32_t portId)
{
    return m_egressStatus[portId];
}

/*-----------------------------------------UbRoundRobinAllocator----------------------------------------------*/
TypeId UbRoundRobinAllocator::GetTypeId(void)
{
    static TypeId tid = TypeId("ns3::UbRoundRobinAllocator")
        .SetParent<UbSwitchAllocator>()
        .AddConstructor<UbRoundRobinAllocator>();
    return tid;
}

void UbRoundRobinAllocator::Init()
{
    UbSwitchAllocator::Init();
    auto node = NodeList::GetNode(m_nodeId);
    uint32_t portsNum = node->GetNDevices();
    auto vlNum = node->GetObject<UbSwitch>()->GetVLNum();
    m_rrIdx.resize(portsNum);
    for (auto &v: m_rrIdx) {
        v.resize(vlNum, 0);
    }
    m_ingressSources.resize(portsNum);
    m_isRunning.resize(portsNum, false);
    m_oneMoreRound.resize(portsNum, false);
    for (auto &i : m_ingressSources) {
        i.resize(vlNum);
    }
}

void UbRoundRobinAllocator::TriggerAllocator(Ptr<UbPort> outPort)
{
    NS_LOG_DEBUG("[UbRoundRobinAllocator TriggerAllocator] portId: " << outPort->GetIfIndex());
    auto outPortId = outPort->GetIfIndex();
    if (m_isRunning[outPortId]) {
        // one more round flag
        // 为了避免running过程中新生成的包：
        // 1.无法被当前轮次调度
        // 2.下一次trigger会被当前轮次的状态掩盖
        m_oneMoreRound[outPortId] = true;
        NS_LOG_DEBUG("[UbRoundRobinAllocator TriggerAllocator] Allocator is running, will retrigger.");
        return;
    }
    m_isRunning[outPortId] = true;
    Simulator::Schedule(m_allocationTime, &UbRoundRobinAllocator::AllocateNextPacket, this, outPort);
}

void UbRoundRobinAllocator::AllocateNextPacket(Ptr<UbPort> outPort)
{
    // 轮询调度
    NS_LOG_DEBUG("[UbRoundRobinAllocator AllocateNextPacket] portId: " << outPort->GetIfIndex());
    auto outPortId = outPort->GetIfIndex();
    auto ingressQueue = SelectNextIngressQueue(outPort);
    // 调度得到的ingressqueue加入egressqueue
    if (ingressQueue != nullptr) {
        auto packet = ingressQueue->GetNextPacket();
        auto inPortId = ingressQueue->GetInPortId();
        auto priority = ingressQueue->GetIngressPriority();
        auto packetEntry = std::make_tuple(inPortId, priority, packet);
        outPort->GetFlowControl()->HandleSentPacket(packet, ingressQueue);
        outPort->GetUbQueue()->DoEnqueue(packetEntry);
        
        // Packet moved from VOQ to EgressQueue, notify Switch to update buffer statistics
        if (inPortId != outPortId) { // Forwarded packet (not locally generated)
            auto node = NodeList::GetNode(m_nodeId);
            node->GetObject<UbSwitch>()->NotifySwitchDequeue(inPortId, outPortId, priority, packet);
        }
    }
    m_isRunning[outPortId] = false;
    // 通知port发包
    Simulator::ScheduleNow(&UbPort::NotifyAllocationFinish, outPort);
    if (m_oneMoreRound[outPortId] == true) {
        m_oneMoreRound[outPortId] = false;
        Simulator::ScheduleNow(&UbRoundRobinAllocator::TriggerAllocator, this, outPort);
        NS_LOG_DEBUG("[UbRoundRobinAllocator AllocateNextPacket] ReTriggerAllocator portId: " << outPort->GetIfIndex());
        return;
    }
}

Ptr<UbIngressQueue> UbRoundRobinAllocator::SelectNextIngressQueue(Ptr<UbPort> outPort)
{
    uint32_t idx;
    uint32_t pi;
    uint32_t outPortId = outPort->GetIfIndex();
    auto node = NodeList::GetNode(m_nodeId);
    auto vlNum = node->GetObject<UbSwitch>()->GetVLNum();
    for (pi = 0 ; pi < vlNum; pi++) {
        size_t qSize = m_ingressSources[outPortId][pi].size();
        for (idx = 0; idx < qSize; idx++) {
            auto qidx = (idx + m_rrIdx[outPortId][pi]) % qSize;
            if (!m_ingressSources[outPortId][pi][qidx]->IsEmpty() &&
                !m_ingressSources[outPortId][pi][qidx]->IsLimited() &&
                !outPort->GetFlowControl()->IsFcLimited(m_ingressSources[outPortId][pi][qidx])) {
                m_rrIdx[outPortId][pi] = (qidx + 1) % qSize;
                NS_LOG_DEBUG("[UbSwitchAllocator DispatchPacket] " << " NodeId: " << node->GetId()
                << " PortId: " << outPortId <<" qidx: "<< qidx);
                return m_ingressSources[outPortId][pi][qidx];
            }
        }
    }
    return nullptr;
}

} // namespae ns3
