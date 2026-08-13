#include <chrono>
#include <cstdlib>

#include <gtest/gtest.h>

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "EquinoxLogger.h"

namespace {

    constexpr int kBenchmarkIterations = 20000;
    constexpr const char* kBenchmarkLogPath = "/tmp/equinox_logger_benchmark.log";

    std::string BuildBenchmarkMessage(std::size_t length) {
        std::string message;
        message.reserve(length);

        while (message.size() < length) {
            message += "benchmark-message-0123456789-";
        }

        if (message.size() > length) {
            message.resize(length);
        }

        return message;
    }

    void EmitBenchmarkRecord(const std::string& testName, const std::string& sinkName, int messageLength, int iterations, double elapsedMs) {
        const double elapsedSeconds = elapsedMs / 1000.0;
        const double logsPerSecond = elapsedSeconds > 0.0 ? (static_cast<double>(iterations) / elapsedSeconds) : 0.0;

        std::cerr << "BENCHMARK_RESULT: {\"test\":\"" << testName << "\",\"sink\":\"" << sinkName << "\",\"message_length\":" << messageLength
                  << ",\"iterations\":" << iterations << ",\"elapsed_ms\":" << elapsedMs << ",\"logs_per_second\":" << logsPerSecond << "}" << std::endl;
    }

    bool IsCiEnvironment() {
        return std::getenv("CI") != nullptr || std::getenv("GITHUB_ACTIONS") != nullptr;
    }

    void RunSinkThroughputBenchmarkImpl(const std::string& sinkName, equinox::logs_output::SINK sinkType, const std::string& baseFilePath,
                                        const std::vector<int>& messageLengths, int iterationsPerCase) {
        for (const int messageLength : messageLengths) {
            const std::string logFilePath = baseFilePath + "_" + std::to_string(messageLength) + ".log";
            std::filesystem::remove(logFilePath);

            ASSERT_TRUE(equinox::setup(equinox::level::LOG_LEVEL::info, "SinkBenchmark", sinkType, logFilePath, 64U * 1024U * 1024U, 2U));

            const std::string message = BuildBenchmarkMessage(static_cast<std::size_t>(messageLength));
            const auto start = std::chrono::steady_clock::now();

            for (int i = 0; i < iterationsPerCase; ++i) {
                equinox::info("%s | idx=%d | sink=%s", message.c_str(), i, sinkName.c_str());
            }

            equinox::flush();

            const auto end = std::chrono::steady_clock::now();
            const double elapsedMs = std::chrono::duration<double, std::milli>(end - start).count();
            EmitBenchmarkRecord("LogThroughputBySinkAndMessageLength", sinkName, messageLength, iterationsPerCase, elapsedMs);
        }
    }

    void RunSinkThroughputBenchmark(const std::string& sinkName, equinox::logs_output::SINK sinkType, const std::string& baseFilePath) {
        const std::vector<int> messageLengths{32, 128, 512, 2048};
        const int iterationsPerCase = 2000;
        RunSinkThroughputBenchmarkImpl(sinkName, sinkType, baseFilePath, messageLengths, iterationsPerCase);
    }

    void RunCiQuietSinkThroughputBenchmark(const std::string& sinkName, equinox::logs_output::SINK sinkType, const std::string& baseFilePath) {
        const std::vector<int> messageLengths{32, 128};
        const int iterationsPerCase = 100;
        RunSinkThroughputBenchmarkImpl(sinkName, sinkType, baseFilePath, messageLengths, iterationsPerCase);
    }

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
        const auto elapsedMs = std::chrono::duration<double, std::milli>(end - start).count();

        std::cerr << "BENCHMARK_RESULT: {\"test\":\"LogPrintThroughput\",\"iterations\":" << kBenchmarkIterations << ",\"elapsed_ms\":" << elapsedMs << "}"
                  << std::endl;

        EXPECT_GT(elapsedMs, 0.0);
    }

    TEST(PerformanceBenchmark, ThroughputBySinkAndMessageLength_Console) {
        RunSinkThroughputBenchmark("console", equinox::logs_output::SINK::console, "/tmp/equinox_logger_sink_console");
    }

    TEST(PerformanceBenchmark, ThroughputBySinkAndMessageLength_File) {
        RunSinkThroughputBenchmark("file", equinox::logs_output::SINK::file, "/tmp/equinox_logger_sink_file");
    }

    TEST(PerformanceBenchmark, ThroughputBySinkAndMessageLength_ConsoleAndFile) {
        RunSinkThroughputBenchmark("console_and_file", equinox::logs_output::SINK::console_and_file, "/tmp/equinox_logger_sink_both");
    }

}  // namespace
