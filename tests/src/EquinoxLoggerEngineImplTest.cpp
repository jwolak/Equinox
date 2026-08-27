#include <gtest/gtest.h>

#include <string>

#include "AsyncLogQueueEngineMock.h"
#include "ConfigFileProviderMock.h"
#include "EquinoxLoggerEngineImpl.h"
#include "FileLogsProducerMock.h"
#include "TimestampProducerMock.h"

namespace equinox_logger_engine_impl_test {

    using namespace testing;
    using namespace equinox;
    using namespace mocks;

    namespace {
        constexpr const char* kFormattedOutputMessage = "Test log";
        constexpr const char* kLogPrefix = "TestPrefix";
        constexpr const char* kExpectedLogPrefix = "[TestPrefix]";
        constexpr const char* kLogFileName = "test.log";
        constexpr std::size_t kMaxLogFileSizeBytes = 4096;
        constexpr std::size_t kMaxLogFiles = 7;
        constexpr const char* kConfigFilePath = "config_file.txt";

        struct LogLevelTestCase {
            level::LOG_LEVEL level;
            const char* expectedMessage;
            bool shouldProcess;
            const char* testName;
        };

        struct SetupLogLevelTestCase {
            level::LOG_LEVEL level;
            const char* testName;
        };

        struct SetupSinkTestCase {
            logs_output::SINK sink;
            bool shouldSetupFile;
            const char* testName;
        };

        struct ChangeLevelTestCase {
            level::LOG_LEVEL level;
            const char* testName;
        };

        struct SetupFromConfigFileTestCase {
            logs_output::SINK sink;
            bool shouldSetupFile;
            bool fileSetupSucceeds;
            bool expectedResult;
            const char* testName;
        };

        std::string GetLogLevelTestCaseName(const TestParamInfo<LogLevelTestCase>& info) {
            return info.param.testName;
        }

        std::string GetSetupLogLevelTestCaseName(const TestParamInfo<SetupLogLevelTestCase>& info) {
            return info.param.testName;
        }

        std::string GetSetupSinkTestCaseName(const TestParamInfo<SetupSinkTestCase>& info) {
            return info.param.testName;
        }

        std::string GetChangeLevelTestCaseName(const TestParamInfo<ChangeLevelTestCase>& info) {
            return info.param.testName;
        }

        std::string GetSetupFromConfigFileTestCaseName(const TestParamInfo<SetupFromConfigFileTestCase>& info) {
            return info.param.testName;
        }
    }  // namespace

    class EquinoxLoggerEngineImplTestable : public EquinoxLoggerEngineImpl {
       public:
        EquinoxLoggerEngineImplTestable(std::shared_ptr<ITimestampProducer> mTimestampProducer, std::shared_ptr<IFileLogsProducer> mFileLogsProducer,
                                        std::unique_ptr<IAsyncLogQueueEngine> mAsyncLogQueueEngine, std::unique_ptr<IConfigFileProvider> mConfigFileProvider)
            : EquinoxLoggerEngineImpl(mTimestampProducer, mFileLogsProducer, std::move(mAsyncLogQueueEngine), std::move(mConfigFileProvider)) {}

        const std::string& getLogPrefixForTests() const {
            return getLogPrefix();
        }

        level::LOG_LEVEL getLogLevelForTests() const {
            return getLogLevel();
        }

        const std::string& getLogFileNameForTests() const {
            return getLogFileName();
        }

        std::size_t getMaxLogFileSizeBytesForTests() const {
            return getMaxLogFileSizeBytes();
        }

        std::size_t getMaxLogFilesForTests() const {
            return getMaxLogFiles();
        }
    };

    class EquinoxLoggerEngineImplTest : public Test {
       public:
        EquinoxLoggerEngineImplTest()
            : timestamp_producer_mock{new StrictMock<TimestampProducerMock>()},
              file_logs_producer_mock{new StrictMock<FileLogsProducerMock>()},
              async_log_queue_engine_mock{new StrictMock<AsyncLogQueueEngineMock>()},
              config_file_provider_mock{new StrictMock<ConfigFileProviderMock>()},
              equinox_Logger_engine_impl{
                  std::shared_ptr<ITimestampProducer>(timestamp_producer_mock), std::shared_ptr<IFileLogsProducer>(file_logs_producer_mock),
                  std::unique_ptr<IAsyncLogQueueEngine>(async_log_queue_engine_mock), std::unique_ptr<IConfigFileProvider>(config_file_provider_mock)} {}

        StrictMock<TimestampProducerMock>* timestamp_producer_mock;
        StrictMock<FileLogsProducerMock>* file_logs_producer_mock;
        StrictMock<AsyncLogQueueEngineMock>* async_log_queue_engine_mock;
        StrictMock<ConfigFileProviderMock>* config_file_provider_mock;
        EquinoxLoggerEngineImplTestable equinox_Logger_engine_impl;
    };

    class EquinoxLoggerEngineImplParameterizedTest : public EquinoxLoggerEngineImplTest, public WithParamInterface<LogLevelTestCase> {};
    class EquinoxLoggerEngineImplSetupLogLevelParameterizedTest : public EquinoxLoggerEngineImplTest, public WithParamInterface<SetupLogLevelTestCase> {};
    class EquinoxLoggerEngineImplSetupSinkParameterizedTest : public EquinoxLoggerEngineImplTest, public WithParamInterface<SetupSinkTestCase> {};
    class EquinoxLoggerEngineImplChangeLevelParameterizedTest : public EquinoxLoggerEngineImplTest, public WithParamInterface<ChangeLevelTestCase> {};
    class EquinoxLoggerEngineImplChangeSinkParameterizedTest : public EquinoxLoggerEngineImplTest, public WithParamInterface<SetupSinkTestCase> {};
    class EquinoxLoggerEngineImplSetupFromConfigFileParameterizedTest : public EquinoxLoggerEngineImplTest,
                                                                        public WithParamInterface<SetupFromConfigFileTestCase> {};

    TEST_P(EquinoxLoggerEngineImplParameterizedTest, Log_Message_For_All_Log_Levels_Is_Processed_According_To_Level) {
        const LogLevelTestCase testCase = GetParam();

        equinox_Logger_engine_impl.changeLevel(level::LOG_LEVEL::trace);
        const std::string expectedMessage = std::string(kLogDefaultPrefix) + testCase.expectedMessage;

        if (testCase.shouldProcess) {
            EXPECT_CALL(*async_log_queue_engine_mock, startWorkerIfNeeded()).Times(1);
            EXPECT_CALL(*async_log_queue_engine_mock, processLogMessage(expectedMessage)).Times(1);
        } else {
            EXPECT_CALL(*async_log_queue_engine_mock, startWorkerIfNeeded()).Times(0);
            EXPECT_CALL(*async_log_queue_engine_mock, processLogMessage(_)).Times(0);
        }

        equinox_Logger_engine_impl.logMessage(testCase.level, kFormattedOutputMessage);
    }

    INSTANTIATE_TEST_SUITE_P(AllLogLevels, EquinoxLoggerEngineImplParameterizedTest,
                             Values(LogLevelTestCase{level::LOG_LEVEL::trace, "[TRACE] Test log", true, "Trace"},
                                    LogLevelTestCase{level::LOG_LEVEL::debug, "[DEBUG] Test log", true, "Debug"},
                                    LogLevelTestCase{level::LOG_LEVEL::info, "[INFO] Test log", true, "Info"},
                                    LogLevelTestCase{level::LOG_LEVEL::warning, "[WARNING] Test log", true, "Warning"},
                                    LogLevelTestCase{level::LOG_LEVEL::error, "[ERROR] Test log", true, "Error"},
                                    LogLevelTestCase{level::LOG_LEVEL::critical, "[CRITICAL] Test log", true, "Critical"},
                                    LogLevelTestCase{level::LOG_LEVEL::off, "", false, "Off"}),
                             GetLogLevelTestCaseName);

    TEST_P(EquinoxLoggerEngineImplSetupLogLevelParameterizedTest, Setup_Logger_For_All_Log_Levels) {
        const SetupLogLevelTestCase testCase = GetParam();

        EXPECT_CALL(*async_log_queue_engine_mock, stopWorker()).Times(1);
        EXPECT_CALL(*async_log_queue_engine_mock, setLogsOutputSink(logs_output::SINK::console)).Times(1);
        EXPECT_CALL(*file_logs_producer_mock, setupFile(_, _, _)).Times(0);
        EXPECT_CALL(*async_log_queue_engine_mock, startWorkerIfNeeded()).Times(1);

        const bool setupResult =
            equinox_Logger_engine_impl.setup(testCase.level, kLogPrefix, logs_output::SINK::console, kLogFileName, kMaxLogFileSizeBytes, kMaxLogFiles);

        EXPECT_TRUE(setupResult);
        EXPECT_EQ(equinox_Logger_engine_impl.getLogLevelForTests(), testCase.level);
        EXPECT_EQ(equinox_Logger_engine_impl.getLogPrefixForTests(), kExpectedLogPrefix);
        EXPECT_EQ(equinox_Logger_engine_impl.getLogFileNameForTests(), kLogFileName);
        EXPECT_EQ(equinox_Logger_engine_impl.getMaxLogFileSizeBytesForTests(), kMaxLogFileSizeBytes);
        EXPECT_EQ(equinox_Logger_engine_impl.getMaxLogFilesForTests(), kMaxLogFiles);
    }

    INSTANTIATE_TEST_SUITE_P(AllSetupLogLevels, EquinoxLoggerEngineImplSetupLogLevelParameterizedTest,
                             Values(SetupLogLevelTestCase{level::LOG_LEVEL::trace, "Trace"}, SetupLogLevelTestCase{level::LOG_LEVEL::debug, "Debug"},
                                    SetupLogLevelTestCase{level::LOG_LEVEL::info, "Info"}, SetupLogLevelTestCase{level::LOG_LEVEL::warning, "Warning"},
                                    SetupLogLevelTestCase{level::LOG_LEVEL::error, "Error"}, SetupLogLevelTestCase{level::LOG_LEVEL::critical, "Critical"},
                                    SetupLogLevelTestCase{level::LOG_LEVEL::off, "Off"}),
                             GetSetupLogLevelTestCaseName);

    TEST_P(EquinoxLoggerEngineImplSetupSinkParameterizedTest, Setup_Logger_For_All_Output_Sinks) {
        const SetupSinkTestCase testCase = GetParam();

        EXPECT_CALL(*async_log_queue_engine_mock, stopWorker()).Times(1);
        EXPECT_CALL(*async_log_queue_engine_mock, setLogsOutputSink(testCase.sink)).Times(1);
        EXPECT_CALL(*async_log_queue_engine_mock, startWorkerIfNeeded()).Times(1);

        if (testCase.shouldSetupFile) {
            EXPECT_CALL(*file_logs_producer_mock, setupFile(kLogFileName, kMaxLogFileSizeBytes, kMaxLogFiles)).Times(1);
        } else {
            EXPECT_CALL(*file_logs_producer_mock, setupFile(_, _, _)).Times(0);
        }

        const bool setupResult =
            equinox_Logger_engine_impl.setup(level::LOG_LEVEL::info, kLogPrefix, testCase.sink, kLogFileName, kMaxLogFileSizeBytes, kMaxLogFiles);

        EXPECT_TRUE(setupResult);
        EXPECT_EQ(equinox_Logger_engine_impl.getLogLevelForTests(), level::LOG_LEVEL::info);
        EXPECT_EQ(equinox_Logger_engine_impl.getLogPrefixForTests(), kExpectedLogPrefix);
        EXPECT_EQ(equinox_Logger_engine_impl.getLogFileNameForTests(), kLogFileName);
        EXPECT_EQ(equinox_Logger_engine_impl.getMaxLogFileSizeBytesForTests(), kMaxLogFileSizeBytes);
        EXPECT_EQ(equinox_Logger_engine_impl.getMaxLogFilesForTests(), kMaxLogFiles);
    }

    INSTANTIATE_TEST_SUITE_P(AllOutputSinks, EquinoxLoggerEngineImplSetupSinkParameterizedTest,
                             Values(SetupSinkTestCase{logs_output::SINK::console, false, "Console"}, SetupSinkTestCase{logs_output::SINK::file, true, "File"},
                                    SetupSinkTestCase{logs_output::SINK::console_and_file, true, "ConsoleAndFile"}),
                             GetSetupSinkTestCaseName);

    TEST_P(EquinoxLoggerEngineImplChangeLevelParameterizedTest, Change_Log_Level_For_All_Levels) {
        const ChangeLevelTestCase testCase = GetParam();

        equinox_Logger_engine_impl.changeLevel(testCase.level);

        EXPECT_EQ(equinox_Logger_engine_impl.getLogLevelForTests(), testCase.level);
    }

    INSTANTIATE_TEST_SUITE_P(AllChangeLevels, EquinoxLoggerEngineImplChangeLevelParameterizedTest,
                             Values(ChangeLevelTestCase{level::LOG_LEVEL::trace, "Trace"}, ChangeLevelTestCase{level::LOG_LEVEL::debug, "Debug"},
                                    ChangeLevelTestCase{level::LOG_LEVEL::info, "Info"}, ChangeLevelTestCase{level::LOG_LEVEL::warning, "Warning"},
                                    ChangeLevelTestCase{level::LOG_LEVEL::error, "Error"}, ChangeLevelTestCase{level::LOG_LEVEL::critical, "Critical"},
                                    ChangeLevelTestCase{level::LOG_LEVEL::off, "Off"}),
                             GetChangeLevelTestCaseName);

    TEST_P(EquinoxLoggerEngineImplChangeSinkParameterizedTest, Change_Logs_Output_Sink_For_All_Sinks) {
        const SetupSinkTestCase testCase = GetParam();

        EXPECT_CALL(*async_log_queue_engine_mock, stopWorker()).Times(1);
        EXPECT_CALL(*async_log_queue_engine_mock, setLogsOutputSink(logs_output::SINK::console)).Times(1);
        EXPECT_CALL(*file_logs_producer_mock, setupFile(_, _, _)).Times(0);
        EXPECT_CALL(*async_log_queue_engine_mock, startWorkerIfNeeded()).Times(1);

        ASSERT_TRUE(
            equinox_Logger_engine_impl.setup(level::LOG_LEVEL::info, kLogPrefix, logs_output::SINK::console, kLogFileName, kMaxLogFileSizeBytes, kMaxLogFiles));

        Mock::VerifyAndClearExpectations(async_log_queue_engine_mock);
        Mock::VerifyAndClearExpectations(file_logs_producer_mock);

        EXPECT_CALL(*async_log_queue_engine_mock, stopWorker()).Times(1);
        EXPECT_CALL(*async_log_queue_engine_mock, setLogsOutputSink(testCase.sink)).Times(1);

        if (testCase.shouldSetupFile) {
            EXPECT_CALL(*file_logs_producer_mock, setupFile(kLogFileName, kMaxLogFileSizeBytes, kMaxLogFiles)).Times(1);
        } else {
            EXPECT_CALL(*file_logs_producer_mock, setupFile(_, _, _)).Times(0);
        }

        EXPECT_CALL(*async_log_queue_engine_mock, startWorkerIfNeeded()).Times(1);

        const bool changeResult = equinox_Logger_engine_impl.changeLogsOutputSink(testCase.sink);

        EXPECT_TRUE(changeResult);
    }

    INSTANTIATE_TEST_SUITE_P(AllChangeOutputSinks, EquinoxLoggerEngineImplChangeSinkParameterizedTest,
                             Values(SetupSinkTestCase{logs_output::SINK::console, false, "Console"}, SetupSinkTestCase{logs_output::SINK::file, true, "File"},
                                    SetupSinkTestCase{logs_output::SINK::console_and_file, true, "ConsoleAndFile"}),
                             GetSetupSinkTestCaseName);

    TEST_F(EquinoxLoggerEngineImplTest, Flush_Method_Calls_Flush_From_AsyncLogQueueEngine) {
        EXPECT_CALL(*async_log_queue_engine_mock, flush()).Times(1);

        equinox_Logger_engine_impl.flush();
    }

    TEST_P(EquinoxLoggerEngineImplSetupFromConfigFileParameterizedTest, Setup_From_Config_File_For_All_Sink_And_File_Setup_Combinations) {
        const SetupFromConfigFileTestCase testCase = GetParam();

        LoggerConfig loaded_config{};
        loaded_config.logLevel = level::LOG_LEVEL::info;
        loaded_config.logPrefix = kLogPrefix;
        loaded_config.logsOutputSink = testCase.sink;
        loaded_config.logFileName = kLogFileName;
        loaded_config.maxLogFileSizeBytes = kMaxLogFileSizeBytes;
        loaded_config.maxLogFiles = kMaxLogFiles;

        EXPECT_CALL(*async_log_queue_engine_mock, stopWorker()).Times(1);
        EXPECT_CALL(*config_file_provider_mock, loadConfigFromFile(kConfigFilePath)).WillOnce(Return(loaded_config));

        if (testCase.shouldSetupFile) {
            if (testCase.fileSetupSucceeds) {
                EXPECT_CALL(*file_logs_producer_mock, setupFile(kLogFileName, kMaxLogFileSizeBytes, kMaxLogFiles)).Times(1);
            } else {
                EXPECT_CALL(*file_logs_producer_mock, setupFile(kLogFileName, kMaxLogFileSizeBytes, kMaxLogFiles))
                    .WillOnce(Throw(std::runtime_error("Failed to setup log file")));
            }
        } else {
            EXPECT_CALL(*file_logs_producer_mock, setupFile(_, _, _)).Times(0);
        }

        EXPECT_CALL(*async_log_queue_engine_mock, startWorkerIfNeeded()).Times(testCase.expectedResult ? 1 : 0);

        EXPECT_EQ(equinox_Logger_engine_impl.setupFromConfigFile(kConfigFilePath), testCase.expectedResult);
    }

    INSTANTIATE_TEST_SUITE_P(AllSetupFromConfigFileCombinations, EquinoxLoggerEngineImplSetupFromConfigFileParameterizedTest,
                             Values(SetupFromConfigFileTestCase{logs_output::SINK::console, false, true, true, "Console"},
                                    SetupFromConfigFileTestCase{logs_output::SINK::file, true, true, true, "File_Success"},
                                    SetupFromConfigFileTestCase{logs_output::SINK::file, true, false, false, "File_Failure"},
                                    SetupFromConfigFileTestCase{logs_output::SINK::console_and_file, true, true, true, "ConsoleAndFile_Success"},
                                    SetupFromConfigFileTestCase{logs_output::SINK::console_and_file, true, false, false, "ConsoleAndFile_Failure"}),
                             GetSetupFromConfigFileTestCaseName);

    TEST_F(EquinoxLoggerEngineImplTest, Setup_From_Config_File_With_Empty_Path_Returns_False) {
        EXPECT_CALL(*async_log_queue_engine_mock, stopWorker()).Times(1);
        EXPECT_CALL(*config_file_provider_mock, loadConfigFromFile(_)).Times(0);
        EXPECT_CALL(*async_log_queue_engine_mock, startWorkerIfNeeded()).Times(0);

        EXPECT_FALSE(equinox_Logger_engine_impl.setupFromConfigFile(""));
    }

}  // namespace equinox_logger_engine_impl_test