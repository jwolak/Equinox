#include <chrono>

#include <gtest/gtest.h>

#include <algorithm>
#include <atomic>
#include <thread>
#include <vector>

#include "EquinoxLoggerEngine.h"
#include "EquinoxLoggerEngineImplMock.h"

namespace equinox_logger_engine_impl_test {
    using namespace equinox;
    using namespace mocks;
    using namespace testing;

    namespace {
        const std::string kTestLogFileName = "test.log";
        const std::size_t kTestMaxLogFileSizeBytes = 1024 * 1024;  // 1 MB
        const std::size_t kTestMaxLogFiles = 5;
        const std::string kTestLogPrefix = "TestPrefix";
    }  // namespace

    class EquinoxLoggerEngineTestable : public EquinoxLoggerEngine {
       public:
        EquinoxLoggerEngineTestable(std::unique_ptr<IEquinoxLoggerEngineImpl> mEquinoxLoggerEngineImpl)
            : EquinoxLoggerEngine(std::move(mEquinoxLoggerEngineImpl)) {}
    };

    class EquinoxLoggerEngineTest : public ::testing::Test {
       public:
        EquinoxLoggerEngineTest()
            : equinox_logger_engine_impl_mock{new StrictMock<EquinoxLoggerEngineImplMock>()},
              equinox_logger_engine{std::unique_ptr<IEquinoxLoggerEngineImpl>(equinox_logger_engine_impl_mock)} {}

        StrictMock<EquinoxLoggerEngineImplMock>* equinox_logger_engine_impl_mock;
        EquinoxLoggerEngineTestable equinox_logger_engine;
    };

    class EquinoxLoggerEngineSetupParamTest : public ::testing::TestWithParam<level::LOG_LEVEL> {
       public:
        EquinoxLoggerEngineSetupParamTest()
            : equinox_logger_engine_impl_mock{new StrictMock<EquinoxLoggerEngineImplMock>()},
              equinox_logger_engine{std::unique_ptr<IEquinoxLoggerEngineImpl>(equinox_logger_engine_impl_mock)} {}

        StrictMock<EquinoxLoggerEngineImplMock>* equinox_logger_engine_impl_mock;
        EquinoxLoggerEngineTestable equinox_logger_engine;
    };

    class EquinoxLoggerEngineSinkParamTest : public ::testing::TestWithParam<logs_output::SINK> {
       public:
        EquinoxLoggerEngineSinkParamTest()
            : equinox_logger_engine_impl_mock{new StrictMock<EquinoxLoggerEngineImplMock>()},
              equinox_logger_engine{std::unique_ptr<IEquinoxLoggerEngineImpl>(equinox_logger_engine_impl_mock)} {}

        StrictMock<EquinoxLoggerEngineImplMock>* equinox_logger_engine_impl_mock;
        EquinoxLoggerEngineTestable equinox_logger_engine;
    };

    TEST_F(EquinoxLoggerEngineTest, Call_GetInstance_And_Returns_Same_Instance) {
        EquinoxLoggerEngine& instance1 = EquinoxLoggerEngine::getInstance();
        EquinoxLoggerEngine& instance2 = EquinoxLoggerEngine::getInstance();

        EXPECT_EQ(&instance1, &instance2);
    }

    TEST_F(EquinoxLoggerEngineTest, Call_Log_With_Format_Arguments_And_Verify_LogMessage_Called_With_Formatted_Text) {
        EXPECT_CALL(*equinox_logger_engine_impl_mock, logMessage(level::LOG_LEVEL::info, "Test value: 42")).Times(1);
        EXPECT_CALL(*equinox_logger_engine_impl_mock, shouldLog(level::LOG_LEVEL::info)).WillOnce(Return(true));

        equinox_logger_engine.log(level::LOG_LEVEL::info, "Test %s: %d", "value", 42);
    }

    TEST_F(EquinoxLoggerEngineTest, Call_Log_With_Invalid_Format_And_Verify_LogMessage_Is_Not_Called) {
        EXPECT_CALL(*equinox_logger_engine_impl_mock, shouldLog(level::LOG_LEVEL::error)).WillOnce(Return(true));
        EXPECT_CALL(*equinox_logger_engine_impl_mock, logMessage(_, _)).Times(0);

        equinox_logger_engine.log(level::LOG_LEVEL::error, "%");
    }

    TEST_F(EquinoxLoggerEngineTest, Call_Log_With_Message_Exceeding_Buffer_Size_And_Verify_Message_Is_Truncated_And_LogMessage_Is_Called) {
        const std::string veryLongMessage(5000, 'A');

        EXPECT_CALL(*equinox_logger_engine_impl_mock, shouldLog(level::LOG_LEVEL::warning)).WillOnce(Return(true));
        EXPECT_CALL(*equinox_logger_engine_impl_mock, logMessage(level::LOG_LEVEL::warning, Truly([](const std::string& msg) {
                                                                     return msg.size() == 4095 &&
                                                                         std::all_of(msg.begin(), msg.end(), [](char c) { return c == 'A'; });
                                                                 })))
            .Times(1);

        equinox_logger_engine.log(level::LOG_LEVEL::warning, "%s", veryLongMessage.c_str());
    }

    TEST_F(EquinoxLoggerEngineTest, Call_Log_From_Many_Threads_And_Verify_Mutex_Serializes_LogMessage_Calls) {
        constexpr int kThreadCount = 8;
        std::atomic<int> activeCalls{0};
        std::atomic<int> maxActiveCalls{0};

        EXPECT_CALL(*equinox_logger_engine_impl_mock, shouldLog(level::LOG_LEVEL::info)).Times(kThreadCount).WillRepeatedly(Return(true));
        EXPECT_CALL(*equinox_logger_engine_impl_mock, logMessage(level::LOG_LEVEL::info, _))
            .Times(kThreadCount)
            .WillRepeatedly(Invoke([&](level::LOG_LEVEL, const std::string&) {
                const int nowActive = ++activeCalls;
                int observedMax = maxActiveCalls.load();
                while (nowActive > observedMax && !maxActiveCalls.compare_exchange_weak(observedMax, nowActive)) {
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                --activeCalls;
            }));

        std::vector<std::thread> workers;
        workers.reserve(kThreadCount);
        for (int i = 0; i < kThreadCount; ++i) {
            workers.emplace_back([&]() { equinox_logger_engine.log(level::LOG_LEVEL::info, "msg %d", 123); });
        }

        for (auto& worker : workers) {
            worker.join();
        }

        EXPECT_EQ(maxActiveCalls.load(), 1);
    }

    TEST_F(EquinoxLoggerEngineTest, Call_SetupFromConfigFile_setupFromConfigFile_From_IEquinoxLoggerEngineImpl_Called) {
        std::string config_file_path = "test_config_file_path";
        EXPECT_CALL(*equinox_logger_engine_impl_mock, setupFromConfigFile(config_file_path)).Times(1);

        equinox_logger_engine.setupFromConfigFile(config_file_path);
    }

    TEST_P(EquinoxLoggerEngineSetupParamTest, Call_Setup_With_Various_Levels_Console_Sink_And_Verify_Parameters_And_Returns_True) {
        const auto logLevel = GetParam();
        EXPECT_CALL(*equinox_logger_engine_impl_mock,
                    setup(logLevel, kTestLogPrefix, logs_output::SINK::console, kTestLogFileName, kTestMaxLogFileSizeBytes, kTestMaxLogFiles))
            .Times(1)
            .WillOnce(Return(true));

        EXPECT_TRUE(
            equinox_logger_engine.setup(logLevel, kTestLogPrefix, logs_output::SINK::console, kTestLogFileName, kTestMaxLogFileSizeBytes, kTestMaxLogFiles));
    }

    INSTANTIATE_TEST_SUITE_P(SetupWithAllLogLevels, EquinoxLoggerEngineSetupParamTest,
                             Values(level::LOG_LEVEL::trace, level::LOG_LEVEL::debug, level::LOG_LEVEL::info, level::LOG_LEVEL::warning,
                                    level::LOG_LEVEL::error, level::LOG_LEVEL::critical));

    TEST_P(EquinoxLoggerEngineSetupParamTest, Try_Setup_But_EquinoxLoggerEngineImpl_Returns_False_And_False_Is_Returned) {
        const auto logLevel = GetParam();
        EXPECT_CALL(*equinox_logger_engine_impl_mock,
                    setup(logLevel, kTestLogPrefix, logs_output::SINK::console, kTestLogFileName, kTestMaxLogFileSizeBytes, kTestMaxLogFiles))
            .Times(1)
            .WillOnce(Return(false));

        EXPECT_FALSE(
            equinox_logger_engine.setup(logLevel, kTestLogPrefix, logs_output::SINK::console, kTestLogFileName, kTestMaxLogFileSizeBytes, kTestMaxLogFiles));
    }

    TEST_P(EquinoxLoggerEngineSetupParamTest, Change_Level_And_Verify_ChangeLevel_Called_With_Correct_Parameter) {
        const auto logLevel = GetParam();
        EXPECT_CALL(*equinox_logger_engine_impl_mock, changeLevel(logLevel)).Times(1);

        equinox_logger_engine.changeLevel(logLevel);
    }

    TEST_P(EquinoxLoggerEngineSinkParamTest, Change_Logs_Output_Sink_And_Verify_ChangeLogsOutputSink_Called_With_Correct_Parameter_And_Returns_True) {
        const auto sink = GetParam();
        EXPECT_CALL(*equinox_logger_engine_impl_mock, changeLogsOutputSink(sink)).Times(1).WillOnce(Return(true));
        EXPECT_TRUE(equinox_logger_engine.changeLogsOutputSink(sink));
    }

    INSTANTIATE_TEST_SUITE_P(ChangeLogsOutputSinkWithAllSupportedSinks, EquinoxLoggerEngineSinkParamTest,
                             Values(logs_output::SINK::console, logs_output::SINK::file, logs_output::SINK::console_and_file));

    TEST_F(EquinoxLoggerEngineTest, Call_Flush_And_Verify_Flush_Called) {
        EXPECT_CALL(*equinox_logger_engine_impl_mock, flush()).Times(1);

        equinox_logger_engine.flush();
    }

    TEST_F(EquinoxLoggerEngineTest, Call_Log_Below_Threshold_And_Verify_LogMessage_Is_Not_Formatted_Or_Called) {
        EXPECT_CALL(*equinox_logger_engine_impl_mock, shouldLog(level::LOG_LEVEL::trace)).WillOnce(Return(false));
        EXPECT_CALL(*equinox_logger_engine_impl_mock, logMessage(_, _)).Times(0);

        equinox_logger_engine.log(level::LOG_LEVEL::trace, "Test %s: %d", "value", 42);
    }

}  // namespace equinox_logger_engine_impl_test