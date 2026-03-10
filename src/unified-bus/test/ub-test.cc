// SPDX-License-Identifier: GPL-2.0-only
/**
 * @file ub-test.cc
 * @brief Test suite for the unified-bus module
 * 
 * This file contains unit tests for the unified-bus functionality,
 * including basic object creation, configuration, and core features.
 */

#include "ns3/test.h"
#include "ns3/ub-app.h"
#include "ns3/ub-traffic-gen.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/config.h"
#include "ns3/rng-seed-manager.h"
#include "ns3/node-container.h"
#include "ns3/ub-utils.h"
#include "ns3/ub-controller.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("UbTest");

/**
 * @brief Unified-bus functionality test
 * 
 * Tests basic unified-bus module functionality including:
 * - Object creation and initialization
 * - Singleton pattern verification
 * - Configuration system integration
 * - Basic API functionality
 */
class UbFunctionalityTest : public TestCase
{
public:
    UbFunctionalityTest();
    void DoRun() override;
    
private:
    void DoSetup() override;
    void DoTeardown() override;
};

UbFunctionalityTest::UbFunctionalityTest()
    : TestCase("UnifiedBus - Core functionality test")
{
}

void UbFunctionalityTest::DoSetup()
{
    Config::Reset();
    RngSeedManager::SetSeed(12345);
}

void UbFunctionalityTest::DoTeardown()
{
    // Minimal cleanup
    if (!Simulator::IsFinished()) {
        Simulator::Destroy();
    }
}

void UbFunctionalityTest::DoRun()
{
    NS_LOG_FUNCTION(this);
    
    // Test 1: UbTrafficGen singleton
    UbTrafficGen& gen1 = UbTrafficGen::GetInstance();
    UbTrafficGen& gen2 = UbTrafficGen::GetInstance();
    NS_TEST_ASSERT_MSG_EQ(&gen1, &gen2, "UbTrafficGen should be singleton");
    
    // Test 2: Initial state
    NS_TEST_ASSERT_MSG_EQ(gen1.IsCompleted(), true, "UbTrafficGen should be completed initially");
    
    // Test 3: UbApp creation
    Ptr<UbApp> app = CreateObject<UbApp>();
    NS_TEST_ASSERT_MSG_NE(app, nullptr, "UbApp creation should succeed");
    
    // Test 4: Node creation
    NodeContainer nodes;
    nodes.Create(2);
    NS_TEST_ASSERT_MSG_EQ(nodes.GetN(), 2, "Should create 2 nodes");
    
    // Test 5: Configuration setting (without getting)
    Config::SetDefault("ns3::UbApp::EnableMultiPath", BooleanValue(false));
    Config::SetDefault("ns3::UbPort::UbDataRate", StringValue("400Gbps"));
    
    NS_LOG_INFO("All basic tests completed successfully");
}

class UbCreateNodeSystemIdTest : public TestCase
{
  public:
    UbCreateNodeSystemIdTest()
        : TestCase("UnifiedBus - CreateNode honors systemId column")
    {
    }

    void DoRun() override
    {
        namespace fs = std::filesystem;

        const uint32_t beforeNodes = NodeList::GetNNodes();
        auto uniqueSuffix = std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count());
        fs::path caseDir = fs::temp_directory_path() / ("ub-systemid-test-" + uniqueSuffix);
        std::error_code ec;
        fs::remove_all(caseDir, ec);
        ec.clear();
        fs::create_directories(caseDir, ec);
        NS_TEST_ASSERT_MSG_EQ(ec.value(), 0, "Temporary case directory creation should succeed");

        fs::path nodePath = caseDir / "node.csv";
        std::ofstream nodeFile(nodePath.string());
        nodeFile << "nodeId,nodeType,portNum,forwardDelay,systemId\n";
        nodeFile << "0,DEVICE,1,1ns,0\n";
        nodeFile << "1,SWITCH,2,1ns,1\n";
        nodeFile.close();

        utils::UbUtils::Get()->CreateNode(nodePath.string());

        NS_TEST_ASSERT_MSG_EQ(NodeList::GetNNodes(), beforeNodes + 2, "CreateNode should create 2 nodes");
        NS_TEST_ASSERT_MSG_EQ(NodeList::GetNode(beforeNodes)->GetSystemId(), 0u,
                              "First created node should preserve systemId 0");
        NS_TEST_ASSERT_MSG_EQ(NodeList::GetNode(beforeNodes + 1)->GetSystemId(), 1u,
                              "Second created node should preserve systemId 1");

        fs::remove_all(caseDir, ec);
    }
};

class UbCreateTopoRemoteLinkTest : public TestCase
{
  public:
    UbCreateTopoRemoteLinkTest()
        : TestCase("UnifiedBus - CreateTopo builds remote link across systemId")
    {
    }

    void DoRun() override
    {
#ifdef NS3_MPI
        namespace fs = std::filesystem;

        const uint32_t beforeNodes = NodeList::GetNNodes();
        auto uniqueSuffix = std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count());
        fs::path caseDir = fs::temp_directory_path() / ("ub-remote-link-test-" + uniqueSuffix);
        std::error_code ec;
        fs::remove_all(caseDir, ec);
        ec.clear();
        fs::create_directories(caseDir, ec);
        NS_TEST_ASSERT_MSG_EQ(ec.value(), 0, "Temporary case directory creation should succeed");

        fs::path nodePath = caseDir / "node.csv";
        const uint32_t node0Id = beforeNodes;
        const uint32_t node1Id = beforeNodes + 1;

        std::ofstream nodeFile(nodePath.string());
        nodeFile << "nodeId,nodeType,portNum,forwardDelay,systemId\n";
        nodeFile << node0Id << ",DEVICE,1,1ns,0\n";
        nodeFile << node1Id << ",DEVICE,1,1ns,1\n";
        nodeFile.close();

        fs::path topoPath = caseDir / "topology.csv";
        std::ofstream topoFile(topoPath.string());
        topoFile << "node1,port1,node2,port2,bandwidth,delay\n";
        topoFile << node0Id << ",0," << node1Id << ",0,400Gbps,10ns\n";
        topoFile.close();

        utils::UbUtils::Get()->CreateNode(nodePath.string());
        utils::UbUtils::Get()->CreateTopo(topoPath.string());

        NS_TEST_ASSERT_MSG_EQ(NodeList::GetNNodes(), beforeNodes + 2, "CreateNode should create 2 nodes");

        Ptr<Node> n0 = NodeList::GetNode(beforeNodes);
        Ptr<Node> n1 = NodeList::GetNode(beforeNodes + 1);
        Ptr<UbPort> p0 = DynamicCast<UbPort>(n0->GetDevice(0));
        Ptr<UbPort> p1 = DynamicCast<UbPort>(n1->GetDevice(0));
        Ptr<Channel> channel = p0->GetChannel();

        NS_TEST_ASSERT_MSG_NE(channel, nullptr, "Port channel should be created");
        NS_TEST_ASSERT_MSG_EQ(channel->GetInstanceTypeId().GetName(), std::string("ns3::UbRemoteLink"),
                              "Cross-systemId topology should use UbRemoteLink");
        NS_TEST_ASSERT_MSG_EQ(p0->HasMpiReceive(), true, "Remote link endpoint should enable MPI receive");
        NS_TEST_ASSERT_MSG_EQ(p1->HasMpiReceive(), true, "Remote link endpoint should enable MPI receive");
        NS_TEST_ASSERT_MSG_EQ(p1->GetChannel(), p0->GetChannel(), "Both ports should share the same link");

        fs::remove_all(caseDir, ec);
#else
        NS_TEST_SKIP_MSG("Requires MPI support");
#endif
    }
};

class UbCreateTpPreloadInstancesTest : public TestCase
{
  public:
    UbCreateTpPreloadInstancesTest()
        : TestCase("UnifiedBus - CreateTp preloads TP instances from config")
    {
    }

    void DoRun() override
    {
        namespace fs = std::filesystem;

        const uint32_t beforeNodes = NodeList::GetNNodes();
        auto uniqueSuffix = std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count());
        fs::path caseDir = fs::temp_directory_path() / ("ub-create-tp-test-" + uniqueSuffix);
        std::error_code ec;
        fs::remove_all(caseDir, ec);
        ec.clear();
        fs::create_directories(caseDir, ec);
        NS_TEST_ASSERT_MSG_EQ(ec.value(), 0, "Temporary case directory creation should succeed");

        const uint32_t node0Id = beforeNodes;
        const uint32_t node1Id = beforeNodes + 1;

        fs::path nodePath = caseDir / "node.csv";
        std::ofstream nodeFile(nodePath.string());
        nodeFile << "nodeId,nodeType,portNum,forwardDelay,systemId\n";
        nodeFile << node0Id << ",DEVICE,1,1ns,0\n";
        nodeFile << node1Id << ",DEVICE,1,1ns,1\n";
        nodeFile.close();

        fs::path tpPath = caseDir / "transport_channel.csv";
        std::ofstream tpFile(tpPath.string());
        tpFile << "nodeId1,portId1,tpn1,nodeId2,portId2,tpn2,priority,metric\n";
        tpFile << node0Id << ",0,11," << node1Id << ",0,22,7,1\n";
        tpFile.close();

        utils::UbUtils::Get()->CreateNode(nodePath.string());
        utils::UbUtils::Get()->CreateTp(tpPath.string());

        Ptr<Node> n0 = NodeList::GetNode(beforeNodes);
        Ptr<Node> n1 = NodeList::GetNode(beforeNodes + 1);
        Ptr<UbController> c0 = n0->GetObject<UbController>();
        Ptr<UbController> c1 = n1->GetObject<UbController>();

        NS_TEST_ASSERT_MSG_EQ(c0->IsTPExists(11), true, "Source-side TP should be preloaded from config");
        NS_TEST_ASSERT_MSG_EQ(c1->IsTPExists(22), true, "Destination-side TP should be preloaded from config");

        fs::remove_all(caseDir, ec);
    }
};

class UbTraceDirSetupTest : public TestCase
{
  public:
    UbTraceDirSetupTest()
        : TestCase("UnifiedBus - Trace directory setup tolerates missing runlog")
    {
    }

    void DoRun() override
    {
        namespace fs = std::filesystem;

        auto uniqueSuffix = std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count());
        fs::path caseDir = fs::temp_directory_path() / ("ub-trace-dir-test-" + uniqueSuffix);
        std::error_code ec;
        fs::remove_all(caseDir, ec);
        ec.clear();
        fs::create_directories(caseDir, ec);
        NS_TEST_ASSERT_MSG_EQ(ec.value(), 0, "Temporary case directory creation should succeed");

        fs::path configPath = caseDir / "network_attribute.txt";
        std::string tracePath = utils::UbUtils::PrepareTraceDir(configPath.string());

        NS_TEST_ASSERT_MSG_EQ(fs::exists(caseDir / "runlog"), true, "runlog directory should be created");
        NS_TEST_ASSERT_MSG_EQ(tracePath.empty(), false, "Returned trace path should not be empty");

        std::ofstream staleFile((caseDir / "runlog" / "stale.tr").string());
        staleFile << "stale";
        staleFile.close();

        tracePath = utils::UbUtils::PrepareTraceDir(configPath.string());
        NS_TEST_ASSERT_MSG_EQ(fs::exists(caseDir / "runlog" / "stale.tr"), false, "Existing runlog contents should be removed");
        NS_TEST_ASSERT_MSG_EQ(fs::exists(caseDir / "runlog"), true, "runlog directory should be recreated");

        fs::remove_all(caseDir, ec);
    }
};

/**
 * @brief Unified-bus test suite
 */
class UbTestSuite : public TestSuite
{
public:
    UbTestSuite();
};

UbTestSuite::UbTestSuite()
    : TestSuite("unified-bus", Type::UNIT)
{
    AddTestCase(new UbFunctionalityTest(), TestCase::Duration::QUICK);
    AddTestCase(new UbTraceDirSetupTest(), TestCase::Duration::QUICK);
    AddTestCase(new UbCreateNodeSystemIdTest(), TestCase::Duration::QUICK);
    AddTestCase(new UbCreateTopoRemoteLinkTest(), TestCase::Duration::QUICK);
    AddTestCase(new UbCreateTpPreloadInstancesTest(), TestCase::Duration::QUICK);
}

// Register the test suite
static UbTestSuite g_ubTestSuite;



