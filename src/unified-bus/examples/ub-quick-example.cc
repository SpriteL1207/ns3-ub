// SPDX-License-Identifier: GPL-2.0-only
#include "ns3/ub-utils.h"
#include "ns3/command-line.h"
#include "ns3/node-list.h"
#include "ns3/ub-app.h"
#include "ns3/ub-traffic-gen.h"
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#ifdef NS3_MPI
#include "ns3/mpi-interface.h"
#include <mpi.h>
#endif

#ifdef NS3_MTP
#include "ns3/mtp-interface.h"
#endif

using namespace ns3;

using namespace utils;

std::string FormatTime(double time_us)
{
    double val = time_us;
    const char* unit = " us";
    int precision = 0;
    if (time_us >= 1e6) {
        val = time_us / 1e6;
        unit = " s";
        precision = 6;
    } else if (time_us >= 1e3) {
        val = time_us / 1e3;
        unit = " ms";
        precision = 3;
    }
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << val << unit;
    return oss.str();
}

void CheckNoProgress(double sim_time_us, std::ostringstream& oss)
{
    static uint32_t last_completed_tasks = 0;
    static double last_progress_time_us = 0;
    uint32_t completed_tasks = 0;
    
    // 统计已完成任务数
    for (auto& task : UbTrafficGen::Get()->m_taskStates) {
        if (task.second == UbTrafficGen::TaskState::COMPLETED) {
            completed_tasks++;
        }
    }

    // 如果有新任务完成，更新状态
    if (completed_tasks > last_completed_tasks) {
        last_completed_tasks = completed_tasks;
        last_progress_time_us = sim_time_us;
    }

    // 如果超过10ms没有新任务完成，且总时间超过10ms，提示可能死锁或拥塞
    if (sim_time_us - last_progress_time_us > 10000 && sim_time_us > 10000) {
         oss << " [WARNING: No task completed for " << FormatTime(sim_time_us - last_progress_time_us) << "]";
    }
}

void CheckExampleProcess()
{
    double sim_time_us = Simulator::Now().GetMicroSeconds();
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
    localtime_r(&t, &tm_buf);

    std::ostringstream oss;
    oss << "[" << std::put_time(&tm_buf, "%H:%M:%S") << "] "
        << "Simulation time progress: " << FormatTime(sim_time_us);

    CheckNoProgress(sim_time_us, oss);

    std::cout << "\r" << oss.str() << std::flush;
    if (!UbTrafficGen::Get()->IsCompleted()) {
            Simulator::Schedule(MicroSeconds(100), &CheckExampleProcess);
            return;
    }
    std::cout << std::endl;
    Simulator::Stop();
    return;
}

bool ReadPositiveEnvVar(const char* name)
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

bool DetectMpiWorld()
{
#ifdef NS3_MPI
    return ReadPositiveEnvVar("OMPI_COMM_WORLD_SIZE") || ReadPositiveEnvVar("PMI_SIZE") ||
           ReadPositiveEnvVar("PMIX_SIZE") || ReadPositiveEnvVar("MV2_COMM_WORLD_SIZE");
#else
    return false;
#endif
}

bool PrepareSimulatorMode(bool enableMpi, uint32_t mtpThreads)
{
#ifdef NS3_MPI
    if (enableMpi)
    {
#ifdef NS3_MTP
        if (mtpThreads > 1)
        {
            MtpInterface::Enable(mtpThreads);
            return true;
        }
        else
#endif
        {
            GlobalValue::Bind("SimulatorImplementationType",
                              StringValue("ns3::DistributedSimulatorImpl"));
        }
        return false;
    }
#endif

#ifdef NS3_MTP
    if (mtpThreads > 1)
    {
        Config::SetDefault("ns3::MultithreadedSimulatorImpl::MaxThreads",
                           UintegerValue(mtpThreads));
        GlobalValue::Bind("SimulatorImplementationType",
                          StringValue("ns3::MultithreadedSimulatorImpl"));
        return true;
    }
#endif

    (void)enableMpi;
    (void)mtpThreads;
    return false;
}

void ConfigureCase(const std::string& configPath)
{
    UbUtils::Get()->SetComponentsAttribute(configPath + "/network_attribute.txt");
    UbUtils::Get()->CreateTraceDir();
    UbUtils::Get()->CreateNode(configPath + "/node.csv");
    UbUtils::Get()->CreateTopo(configPath + "/topology.csv");
    UbUtils::Get()->AddRoutingTable(configPath + "/routing_table.csv");
    UbUtils::Get()->CreateTp(configPath + "/transport_channel.csv");
    UbUtils::Get()->TopoTraceConnect();
}

uint32_t ActivateTasks(const std::string& configPath, bool activateLocalOwnedTasksOnly, uint32_t mpiRank)
{
    auto trafficData = UbUtils::Get()->ReadTrafficCSV(configPath + "/traffic.csv");
    BooleanValue faultEnabled;
    UbUtils::Get()->g_fault_enable.GetValue(faultEnabled);
    if (faultEnabled.Get())
    {
        UbUtils::Get()->InitFaultMoudle(configPath + "/fault.csv");
    }

    uint32_t localTaskCount = 0;
    UbUtils::Get()->PrintTimestamp("Start Client.");
    for (const auto& record : trafficData)
    {
        Ptr<Node> sourceNode = NodeList::GetNode(record.sourceNode);
        if (activateLocalOwnedTasksOnly &&
            UbUtils::ExtractMpiRank(sourceNode->GetSystemId()) != mpiRank)
        {
            continue;
        }

        if (sourceNode->GetNApplications() == 0)
        {
            Ptr<UbApp> client = CreateObject<UbApp>();
            sourceNode->AddApplication(client);
            UbUtils::Get()->ClientTraceConnect(record.sourceNode);
        }
        UbTrafficGen::Get()->AddTask(record);
        ++localTaskCount;
    }

    UbTrafficGen::Get()->ScheduleNextTasks();
    CheckExampleProcess();
    return localTaskCount;
}

// 根据配置文件路径执行用例
int main(int argc, char* argv[])
{
    // 先检查是否查询属性信息（使用独立的参数检查，不触发严格解析）
    for (int i = 1; i < argc; ++i) {
        std::string arg(argv[i]);
        if (arg.find("--ClassName") == 0) {
            if (UbUtils::Get()->QueryAttributeInfo(argc, argv))
                return 0;
            break;
        }
    }

    // 多线程加速配置（需编译时启用：./ns3 configure --enable-mtp）
    // 运行时通过 --mtp-threads=N 指定线程数（0-1=禁用，>=2 启用）
    uint32_t mtpThreads = 0;
    std::string casePathArg;
    std::string positionalCasePath;
    CommandLine cmd;
    cmd.AddValue("mtp-threads", "Number of MTP threads (0-1 to disable, >=2 to enable)", mtpThreads);
    cmd.AddValue("case-path", "Path to the unified-bus case directory", casePathArg);
    uint32_t stopMs = 0;
    cmd.AddValue("stop-ms", "Optional simulation stop time in milliseconds", stopMs);
    cmd.AddNonOption("casePath", "Optional unified-bus case directory", positionalCasePath);
    cmd.Parse(argc, argv);

    // 开始计时
    auto start = std::chrono::high_resolution_clock::now();
    Time::SetResolution(Time::PS);

    // 日志中添加时间前缀
    ns3::LogComponentEnableAll(LOG_PREFIX_TIME);

    // 示例：设置指定组件日志级别，设置指定组件打印时间前缀
    // LogComponentEnable("UbApp", LOG_LEVEL_INFO);
    // LogComponentEnable("UbApp", LOG_PREFIX_TIME);

    // 激活日志
    LogComponentEnable("UbSwitchAllocator", LOG_LEVEL_WARN);
    LogComponentEnable("UbQueueManager", LOG_LEVEL_WARN);
    LogComponentEnable("UbCaqm", LOG_LEVEL_WARN);
    LogComponentEnable("UbTrafficGen", LOG_LEVEL_WARN);
    LogComponentEnable("UbApp", LOG_LEVEL_WARN);
    LogComponentEnable("UbCongestionControl", LOG_LEVEL_WARN);
    LogComponentEnable("UbController", LOG_LEVEL_WARN);
    LogComponentEnable("UbDataLink", LOG_LEVEL_WARN);
    LogComponentEnable("UbFlowControl", LOG_LEVEL_WARN);
    LogComponentEnable("UbHeader", LOG_LEVEL_WARN);
    LogComponentEnable("UbLink", LOG_LEVEL_WARN);
    LogComponentEnable("UbLdstInstance", LOG_LEVEL_WARN);
    LogComponentEnable("UbLdstThread", LOG_LEVEL_WARN);
    LogComponentEnable("UbLdstApi", LOG_LEVEL_WARN);
    LogComponentEnable("UbPort", LOG_LEVEL_WARN);
    LogComponentEnable("UbRoutingProcess", LOG_LEVEL_WARN);
    LogComponentEnable("UbSwitch", LOG_LEVEL_WARN);
    LogComponentEnable("UbFunction", LOG_LEVEL_WARN);
    LogComponentEnable("UbTransportChannel", LOG_LEVEL_WARN);
    LogComponentEnable("UbFault", LOG_LEVEL_WARN);
    LogComponentEnable("UbTransaction", LOG_LEVEL_WARN);
    LogComponentEnable("TpConnectionManager", LOG_LEVEL_WARN);

    // 配置文件路径
    string configPath = casePathArg.empty() ? positionalCasePath : casePathArg;
    if (configPath.empty()) {
        configPath = "scratch/2nodes_single-tp";
    }

    // 读取配置文件并执行用例
    string runCase = "Run case: " + configPath;
    UbUtils::Get()->PrintTimestamp(runCase);
    RngSeedManager::SetSeed(10);
    const bool enableMpi = DetectMpiWorld();
    const bool mtpEnabled = PrepareSimulatorMode(enableMpi, mtpThreads);

    uint32_t mpiRank = 0;
#ifdef NS3_MPI
    if (enableMpi)
    {
        MpiInterface::Enable(&argc, &argv);
        mpiRank = MpiInterface::GetSystemId();
    }
#endif

    if (mtpThreads > 1)
    {
        if (mtpEnabled)
        {
            std::cout << "[INFO] MTP enabled with " << mtpThreads << " threads."
                      << (enableMpi ? " (hybrid MPI mode)." : " (local mode).")
                      << std::endl;
        }
        else
        {
            std::cerr << "[WARNING] MTP requested but not compiled. Reconfigure with --enable-mtp"
                      << std::endl;
        }
    }

    auto sim_wall_start = std::chrono::high_resolution_clock::now();
    ConfigureCase(configPath);
    ActivateTasks(configPath, enableMpi, mpiRank);
    if (stopMs > 0)
    {
        Simulator::Stop(MilliSeconds(stopMs));
    }
    Simulator::Run();
    UbUtils::Get()->Destroy();
    Simulator::Destroy();
#ifdef NS3_MPI
    if (enableMpi && MpiInterface::IsEnabled())
    {
        MpiInterface::Disable();
    }
#endif
    auto sim_wall_end = std::chrono::high_resolution_clock::now();
    UbUtils::Get()->PrintTimestamp("Simulator finished!");
    auto trace_wall_start = std::chrono::high_resolution_clock::now();
    UbUtils::Get()->ParseTrace();

    auto end = std::chrono::high_resolution_clock::now();
    UbUtils::Get()->PrintTimestamp("Program finished.");
    // 阶段性挂钟时间统计（单位：秒）
    double config_wall_s = std::chrono::duration_cast<std::chrono::microseconds>(sim_wall_start - start).count() / 1e6;
    double run_wall_s    = std::chrono::duration_cast<std::chrono::microseconds>(sim_wall_end - sim_wall_start).count() / 1e6;
    double trace_wall_s  = std::chrono::duration_cast<std::chrono::microseconds>(end - trace_wall_start).count() / 1e6;
    double total_wall_s  = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1e6;
    UbUtils::Get()->PrintTimestamp("Wall-clock (config phase): " + std::to_string(config_wall_s) + " s");
    UbUtils::Get()->PrintTimestamp("Wall-clock (run phase): " + std::to_string(run_wall_s) + " s");
    UbUtils::Get()->PrintTimestamp("Wall-clock (trace phase): " + std::to_string(trace_wall_s) + " s");
    UbUtils::Get()->PrintTimestamp("Wall-clock (total): " + std::to_string(total_wall_s) + " s");
    return 0;
}
