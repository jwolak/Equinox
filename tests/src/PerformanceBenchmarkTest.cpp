#include <chrono>

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "EquinoxLogger.h"

namespace {

    constexpr int kBenchmarkIterations = 20000;
    constexpr const char* kBenchmarkLogPath = "/tmp/equinox_logger_benchmark.log";

    TEST(PerformanceBenchmark, LogPrintThroughput) {
        std::filesystem::remove(kBenchmarkLogPath);

        ASSERT_TRUE(equinox::setup(equinox::level::LOG_LEVEL::info, "PerformanceBenchmark", equinox::logs_output::SINK::file, kBenchmarkLogPath,
                                   32U * 1024U * 1024U, 2U));

        const auto start = std::chrono::steady_clock::now();

        for (int i = 0; i < kBenchmarkIterations; ++i) {
            equinox::info("benchmark message %d | value=%d | tag=%s", i, i * 11, "perf");
        }

        equinox::flush();

        const auto end = std::chrono::steady_clock::now();
        const auto elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();

        std::cout << "BENCHMARK_RESULT: {\"test\":\"LogPrintThroughput\",\"iterations\":" << kBenchmarkIterations << ",\"elapsed_ms\":" << elapsed_ms << "}"
                  << std::endl;

        EXPECT_GT(elapsed_ms, 0.0);
    }

}  // namespace
