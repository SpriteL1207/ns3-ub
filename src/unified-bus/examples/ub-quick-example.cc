// SPDX-License-Identifier: GPL-2.0-only
/*
 * Unified-bus config-driven user entry.
 *
 * Typical usage:
 *   local:      build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=<case-dir>
 *   MPI:        mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=<case-dir>
 *   MPI + MTP:  mpirun -np 2 build/src/unified-bus/examples/ns3.44-ub-quick-example-default --case-path=<case-dir> --mtp-threads=2
 *
 * This example is the unified config-driven user entry. The separate
 * `ub-mtp-remote-tp-regression` binary remains regression-only.
 */
#include "ns3/ub-utils.h"
#include "ns3/command-line.h"
#include "ns3/node-list.h"
#include "ns3/ub-app.h"
#include "ns3/ub-traffic-gen.h"
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
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

namespace
{

struct QuickExampleOptions
{
    bool test = false;
    uint32_t mtpThreads = 0;
    uint32_t stopMs = 0;
    std::string configPath;
};

struct RuntimeSelection
{
    bool enableMpi = false;
    bool mtpEnabled = false;
    uint32_t mpiRank = 0;
};

struct PhaseTiming
{
    std::chrono::high_resolution_clock::time_point programStart;
    std::chrono::high_resolution_clock::time_point simulationStart;
    std::chrono::high_resolution_clock::time_point simulationEnd;
    std::chrono::high_resolution_clock::time_point traceStart;
    std::chrono::high_resolution_clock::time_point programEnd;
};

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

void PrintTestResult(bool passed, bool enableMpi, uint32_t mpiRank)
{
    if (!passed)
    {
        std::cout << "TEST : 00000 : FAILED" << std::endl;
        return;
    }

#ifdef NS3_MPI
    if (enableMpi && mpiRank != 0)
    {
        return;
    }
#else
    (void)enableMpi;
    (void)mpiRank;
#endif

    std::cout << "TEST : 00000 : PASSED" << std::endl;
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
#else
    if (mtpThreads > 1)
    {
        std::cerr << "[WARNING] MTP requested but not compiled. Reconfigure with --enable-mtp"
                  << std::endl;
    }
#endif

    (void)enableMpi;
    (void)mtpThreads;
    return false;
}

std::string NormalizeCasePath(const std::string& path)
{
    return std::filesystem::absolute(std::filesystem::path(path)).lexically_normal().string();
}

void BuildScenarioFromConfig(const std::string& configPath)
{
    UbUtils::Get()->SetComponentsAttribute(configPath + "/network_attribute.txt");
    UbUtils::Get()->CreateTraceDir();
    UbUtils::Get()->CreateNode(configPath + "/node.csv");
    UbUtils::Get()->CreateTopo(configPath + "/topology.csv");
    UbUtils::Get()->AddRoutingTable(configPath + "/routing_table.csv");
    UbUtils::Get()->CreateTp(configPath + "/transport_channel.csv");
    UbUtils::Get()->TopoTraceConnect();
}

uint32_t ActivateTrafficFromConfig(const std::string& configPath,
                                   bool activateLocalOwnedTasksOnly,
                                   uint32_t mpiRank)
{
    auto trafficData = UbUtils::Get()->ReadTrafficCSV(configPath + "/traffic.csv");
    if (UbUtils::Get()->IsFaultEnabled())
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

bool HandleAttributeQuery(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg(argv[i]);
        if (arg.find("--ClassName") == 0)
        {
            if (UbUtils::Get()->QueryAttributeInfo(argc, argv))
            {
                return true;
            }
            break;
        }
    }
    return false;
}

QuickExampleOptions ParseOptions(int argc, char* argv[])
{
    QuickExampleOptions options;
    std::string casePathArg;
    std::string positionalCasePath;
    CommandLine cmd;
    cmd.AddValue("test", "Enable regression-test style output", options.test);
    cmd.AddValue("mtp-threads",
                 "Number of MTP threads (0-1 to disable, >=2 to enable)",
                 options.mtpThreads);
    cmd.AddValue("case-path",
                 "Required path to the unified-bus case directory",
                 casePathArg);
    cmd.AddValue("stop-ms", "Optional simulation stop time in milliseconds", options.stopMs);
    cmd.AddNonOption("casePath",
                     "Required unified-bus case directory when --case-path is omitted",
                     positionalCasePath);
    cmd.Parse(argc, argv);

    if (!casePathArg.empty() && !positionalCasePath.empty() &&
        NormalizeCasePath(casePathArg) != NormalizeCasePath(positionalCasePath))
    {
        std::cerr << "conflicting case paths provided via --case-path and casePath" << std::endl;
        std::exit(1);
    }

    options.configPath = casePathArg.empty() ? positionalCasePath : casePathArg;
    if (options.configPath.empty())
    {
        std::cerr << "missing required case path (--case-path or casePath)" << std::endl;
        std::exit(1);
    }

    return options;
}

void EnableExampleLogging()
{
    Time::SetResolution(Time::PS);

    ns3::LogComponentEnableAll(LOG_PREFIX_TIME);

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
}

RuntimeSelection PrepareRuntime(int* argc, char*** argv, const QuickExampleOptions& options)
{
    RuntimeSelection runtime;
    runtime.enableMpi = DetectMpiWorld();
    runtime.mtpEnabled = PrepareSimulatorMode(runtime.enableMpi, options.mtpThreads);

#ifdef NS3_MPI
    if (runtime.enableMpi)
    {
        MpiInterface::Enable(argc, argv);
        runtime.mpiRank = MpiInterface::GetSystemId();
    }
#endif

    if (options.mtpThreads > 1)
    {
        if (runtime.mtpEnabled)
        {
            std::cout << "[INFO] MTP enabled with " << options.mtpThreads << " threads."
                      << (runtime.enableMpi ? " (hybrid MPI mode)." : " (local mode).")
                      << std::endl;
        }
        else
        {
            std::cerr << "[WARNING] MTP requested but not compiled. Reconfigure with --enable-mtp"
                      << std::endl;
        }
    }

    return runtime;
}

PhaseTiming RunScenario(const QuickExampleOptions& options,
                        const RuntimeSelection& runtime,
                        const std::chrono::high_resolution_clock::time_point& programStart)
{
    PhaseTiming timing;
    timing.programStart = programStart;

    EnableExampleLogging();

    UbUtils::Get()->PrintTimestamp("Run case: " + options.configPath);
    RngSeedManager::SetSeed(10);

    timing.simulationStart = std::chrono::high_resolution_clock::now();
    BuildScenarioFromConfig(options.configPath);
    ActivateTrafficFromConfig(options.configPath, runtime.enableMpi, runtime.mpiRank);
    if (options.stopMs > 0)
    {
        Simulator::Stop(MilliSeconds(options.stopMs));
    }
    Simulator::Run();
    timing.simulationEnd = std::chrono::high_resolution_clock::now();

    UbUtils::Get()->Destroy();
    Simulator::Destroy();
#ifdef NS3_MPI
    if (runtime.enableMpi && MpiInterface::IsEnabled())
    {
        MpiInterface::Disable();
    }
#endif

    UbUtils::Get()->PrintTimestamp("Simulator finished!");
    timing.traceStart = std::chrono::high_resolution_clock::now();
    UbUtils::Get()->ParseTrace(options.test);
    timing.programEnd = std::chrono::high_resolution_clock::now();
    return timing;
}

void ReportResult(const QuickExampleOptions& options,
                  const RuntimeSelection& runtime,
                  const PhaseTiming& timing)
{
    UbUtils::Get()->PrintTimestamp("Program finished.");
    double config_wall_s =
        std::chrono::duration_cast<std::chrono::microseconds>(timing.simulationStart -
                                                              timing.programStart)
            .count() /
        1e6;
    double run_wall_s =
        std::chrono::duration_cast<std::chrono::microseconds>(timing.simulationEnd -
                                                              timing.simulationStart)
            .count() /
        1e6;
    double trace_wall_s =
        std::chrono::duration_cast<std::chrono::microseconds>(timing.programEnd - timing.traceStart)
            .count() /
        1e6;
    double total_wall_s =
        std::chrono::duration_cast<std::chrono::microseconds>(timing.programEnd - timing.programStart)
            .count() /
        1e6;
    UbUtils::Get()->PrintTimestamp("Wall-clock (config phase): " + std::to_string(config_wall_s) + " s");
    UbUtils::Get()->PrintTimestamp("Wall-clock (run phase): " + std::to_string(run_wall_s) + " s");
    UbUtils::Get()->PrintTimestamp("Wall-clock (trace phase): " + std::to_string(trace_wall_s) + " s");
    UbUtils::Get()->PrintTimestamp("Wall-clock (total): " + std::to_string(total_wall_s) + " s");
    if (options.test)
    {
        PrintTestResult(UbTrafficGen::Get()->IsCompleted(), runtime.enableMpi, runtime.mpiRank);
    }
}

} // namespace

int main(int argc, char* argv[])
{
    if (HandleAttributeQuery(argc, argv))
    {
        return 0;
    }

    QuickExampleOptions options = ParseOptions(argc, argv);
    const auto programStart = std::chrono::high_resolution_clock::now();
    RuntimeSelection runtime = PrepareRuntime(&argc, &argv, options);
    PhaseTiming timing = RunScenario(options, runtime, programStart);
    ReportResult(options, runtime, timing);
    return 0;
}
