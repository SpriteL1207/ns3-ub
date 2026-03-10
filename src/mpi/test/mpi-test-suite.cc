/*
 * Copyright (c) 2018 Lawrence Livermore National Laboratory
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Peter D. Barnes, Jr. <pdbarnes@llnl.gov>
 */

#include "ns3/example-as-test.h"

#include <cstdlib>
#include <fstream>
#include <sstream>

using namespace ns3;

/**
 * @ingroup mpi-tests
 *
 * This version of ns3::ExampleTestCase is specialized for MPI
 * by accepting the number of ranks as a parameter,
 * then building a `--command-template` string which
 * invokes `mpiexec` correctly to execute MPI examples.
 */
class MpiTestCase : public ExampleAsTestCase
{
  public:
    /**
     * @copydoc ns3::ExampleAsTestCase::ExampleAsTestCase
     *
     * @param [in] ranks The number of ranks to use
     */
    MpiTestCase(const std::string name,
                const std::string program,
                const std::string dataDir,
                const int ranks,
                const std::string args = "",
                const bool shouldNotErr = true);

    /** Destructor */
    ~MpiTestCase() override
    {
    }

    /**
     * Produce the `--command-template` argument which will invoke
     * `mpiexec` with the requested number of ranks.
     *
     * @returns The `--command-template` string.
     */
    std::string GetCommandTemplate() const override;

    /**
     * Sort the output from parallel execution.
     * stdout from multiple ranks is not ordered.
     *
     * @returns Sort command
     */
    std::string GetPostProcessingCommand() const override;

  private:
    /** The number of ranks. */
    int m_ranks;
};

MpiTestCase::MpiTestCase(const std::string name,
                         const std::string program,
                         const std::string dataDir,
                         const int ranks,
                         const std::string args /* = "" */,
                         const bool shouldNotErr /* = true */)
    : ExampleAsTestCase(name, program, dataDir, args, shouldNotErr),
      m_ranks(ranks)
{
}

std::string
MpiTestCase::GetCommandTemplate() const
{
    std::stringstream ss;
    ss << "mpiexec -n " << m_ranks << " %s --test " << m_args;
    return ss.str();
}

std::string
MpiTestCase::GetPostProcessingCommand() const
{
    std::string command("| grep TEST | sort ");
    return command;
}

/**
 * @ingroup mpi-tests
 * MPI specialization of ns3::ExampleTestSuite.
 */
class MpiTestSuite : public TestSuite
{
  public:
    /**
     * @copydoc MpiTestCase::MpiTestCase
     *
     * @param [in] duration Amount of time this test takes to execute
     *             (defaults to QUICK).
     */
    MpiTestSuite(const std::string name,
                 const std::string program,
                 const std::string dataDir,
                 const int ranks,
                 const std::string args = "",
                 const Duration duration = Duration::QUICK,
                 const bool shouldNotErr = true)
        : TestSuite(name, Type::EXAMPLE)
    {
        AddTestCase(new MpiTestCase(name, program, dataDir, ranks, args, shouldNotErr), duration);
    }

}; // class MpiTestSuite

class MpiHybridSmokeInterceptorRemovedTestCase : public TestCase
{
  public:
    MpiHybridSmokeInterceptorRemovedTestCase()
        : TestCase("mpi-example-ub-hybrid-smoke-2-interceptor-removed")
    {
    }

    void DoRun() override
    {
        SetDataDir(NS_TEST_SOURCEDIR);
        const std::string testFile = CreateTempDirFilename(GetName() + ".log");
        const std::string command =
            "python3 ./ns3 run ub-hybrid-smoke --no-build "
            "--command-template=\"mpiexec -n 2 %s --test --mode=interceptor\" > " +
            testFile + " 2>&1";

        const int status = std::system(command.c_str());

        std::ifstream input(testFile);
        std::stringstream buffer;
        buffer << input.rdbuf();
        const std::string output = buffer.str();

        NS_TEST_ASSERT_MSG_NE(status,
                              0,
                              "interceptor-removed regression should fail fast when deprecated mode is requested");
        NS_TEST_ASSERT_MSG_NE(output.find("TEST ERROR interceptor mode has been removed; use tp"),
                              std::string::npos,
                              "deprecated mode should emit the expected error message");
    }
};

class MpiHybridSmokeInterceptorRemovedTestSuite : public TestSuite
{
  public:
    MpiHybridSmokeInterceptorRemovedTestSuite()
        : TestSuite("mpi-example-ub-hybrid-smoke-2-interceptor-removed", Type::SYSTEM)
    {
        AddTestCase(new MpiHybridSmokeInterceptorRemovedTestCase(), TestCase::Duration::QUICK);
    }
};

/* Tests using SimpleDistributedSimulatorImpl */
static MpiTestSuite g_mpiNms2("mpi-example-nms-2", "nms-p2p-nix-distributed", NS_TEST_SOURCEDIR, 2);
static MpiTestSuite g_mpiComm2("mpi-example-comm-2",
                               "simple-distributed-mpi-comm",
                               NS_TEST_SOURCEDIR,
                               2);
static MpiTestSuite g_mpiComm2comm("mpi-example-comm-2-init",
                                   "simple-distributed-mpi-comm",
                                   NS_TEST_SOURCEDIR,
                                   2,
                                   "--init");
static MpiTestSuite g_mpiComm3comm("mpi-example-comm-3-init",
                                   "simple-distributed-mpi-comm",
                                   NS_TEST_SOURCEDIR,
                                   3,
                                   "--init");
static MpiTestSuite g_mpiEmpty2("mpi-example-empty-2",
                                "simple-distributed-empty-node",
                                NS_TEST_SOURCEDIR,
                                2);
static MpiTestSuite g_mpiEmpty3("mpi-example-empty-3",
                                "simple-distributed-empty-node",
                                NS_TEST_SOURCEDIR,
                                3);
static MpiTestSuite g_mpiSimple2("mpi-example-simple-2",
                                 "simple-distributed",
                                 NS_TEST_SOURCEDIR,
                                 2);
static MpiTestSuite g_mpiThird2("mpi-example-third-2", "third-distributed", NS_TEST_SOURCEDIR, 2);
static MpiTestSuite g_mpiUbConfigSmoke2("mpi-example-ub-mpi-config-smoke-2",
                                        "ub-mpi-config-smoke",
                                        NS_TEST_SOURCEDIR,
                                        2,
                                        "--case-path=scratch/ub-mpi-minimal");
static MpiTestSuite g_mpiUbConfigTpOwnership2("mpi-example-ub-mpi-config-tp-ownership-2",
                                              "ub-mpi-config-smoke",
                                              NS_TEST_SOURCEDIR,
                                              2,
                                              "--case-path=scratch/ub-mpi-minimal --verify-tp-ownership");

#ifdef NS3_MTP
static MpiTestSuite g_mpiUbConfigHybridSmoke2("mpi-example-ub-mpi-config-hybrid-smoke-2",
                                              "ub-mpi-config-smoke",
                                              NS_TEST_SOURCEDIR,
                                              2,
                                              "--case-path=scratch/ub-mpi-hybrid-minimal --mtp-threads=2 --verify-packed-systemid --verify-tp-ownership --stop-ms=50");
static MpiTestSuite g_mpiUbConfigHybridCbfc2("mpi-example-ub-mpi-config-hybrid-cbfc-2",
                                             "ub-mpi-config-smoke",
                                             NS_TEST_SOURCEDIR,
                                             2,
                                             "--case-path=scratch/ub-mpi-hybrid-cbfc-minimal --mtp-threads=2 --verify-packed-systemid --verify-tp-ownership --verify-cbfc-control --verify-cbfc-control-count --stop-ms=50");
static MpiTestSuite g_mpiUbHybridSmoke2("mpi-example-ub-hybrid-smoke-2",
                                        "ub-hybrid-smoke",
                                        NS_TEST_SOURCEDIR,
                                        2);
static MpiHybridSmokeInterceptorRemovedTestSuite g_mpiUbHybridSmoke2InterceptorRemoved;
#endif

/* Tests using NullMessageSimulatorImpl */
static MpiTestSuite g_mpiSimple2NullMsg("mpi-example-simple-2-nullmsg",
                                        "simple-distributed",
                                        NS_TEST_SOURCEDIR,
                                        2,
                                        "--nullmsg");
static MpiTestSuite g_mpiEmpty2NullMsg("mpi-example-empty-2-nullmsg",
                                       "simple-distributed-empty-node",
                                       NS_TEST_SOURCEDIR,
                                       2,
                                       "-nullmsg");
static MpiTestSuite g_mpiEmpty3NullMsg("mpi-example-empty-3-nullmsg",
                                       "simple-distributed-empty-node",
                                       NS_TEST_SOURCEDIR,
                                       3,
                                       "-nullmsg");
