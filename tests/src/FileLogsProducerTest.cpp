#include <gtest/gtest.h>

#include <memory>

#include "FileLogsProducer.h"
#include "TimestampProducerMock.h"

namespace file_logs_producer_test {
    using namespace equinox;
    using namespace testing;
    using namespace mocks;

    namespace {
        const std::string kTestLogFileName = "test_log.log";
        const std::size_t kTestMaxLogFileSizeBytes = 1024U;
        const std::size_t kTestMaxLogFiles = 5U;
    }  // namespace

    class FileLogsProducerTestable : public FileLogsProducer {
       public:
        FileLogsProducerTestable(std::shared_ptr<ITimestampProducer> timestampProducer) : FileLogsProducer(timestampProducer) {}

        using FileLogsProducer::buildRotatedFileName;
        using FileLogsProducer::GetLogFileName;
        using FileLogsProducer::GetLogFileStream;
        using FileLogsProducer::GetMaxLogFiles;
        using FileLogsProducer::GetMaxLogFileSizeBytes;
        using FileLogsProducer::GetNextRotationIndex;
        using FileLogsProducer::isRotationEnabled;
        using FileLogsProducer::openLogFileAppend;
        using FileLogsProducer::openLogFileTruncate;
        using FileLogsProducer::rotateIfNeeded;
    };

    class FileLogsProducerTest : public Test {
       public:
        FileLogsProducerTest() : timestamp_producer_mock(std::make_shared<StrictMock<TimestampProducerMock>>()), file_logs_producer(timestamp_producer_mock) {}

        std::shared_ptr<StrictMock<TimestampProducerMock>> timestamp_producer_mock;
        FileLogsProducerTestable file_logs_producer;
    };

    TEST_F(FileLogsProducerTest, Setup_File_And_Log_File_Name_Set) {
        file_logs_producer.setupFile(kTestLogFileName, 0U, 0U);

        EXPECT_EQ(file_logs_producer.GetLogFileName(), kTestLogFileName);
    }

    TEST_F(FileLogsProducerTest, Setup_File_And_Max_Log_File_Size_Bytes_Set) {
        file_logs_producer.setupFile(kTestLogFileName, kTestMaxLogFileSizeBytes, 0U);

        EXPECT_EQ(file_logs_producer.GetMaxLogFileSizeBytes(), kTestMaxLogFileSizeBytes);
    }

    TEST_F(FileLogsProducerTest, Setup_File_And_Max_Log_Files_Set) {
        file_logs_producer.setupFile(kTestLogFileName, 0U, kTestMaxLogFiles);

        EXPECT_EQ(file_logs_producer.GetMaxLogFiles(), kTestMaxLogFiles);
    }

    TEST_F(FileLogsProducerTest, Try_Setup_File_But_File_Is_Already_Opened_And_Close_Failed) {
        file_logs_producer.setupFile(kTestLogFileName, 0U, 0U);
        file_logs_producer.GetLogFileStream().exceptions(std::ofstream::failbit | std::ofstream::badbit);
        EXPECT_CALL(*timestamp_producer_mock, getTimestamp()).Times(0);
        EXPECT_CALL(*timestamp_producer_mock, getTimestampInUs()).Times(0);

        EXPECT_NO_THROW(file_logs_producer.setupFile(kTestLogFileName, 0U, 0U));
    }

    TEST_F(FileLogsProducerTest, Try_Setup_File_But_Open_Log_File_Failed) {
        file_logs_producer.GetLogFileStream().exceptions(std::ofstream::failbit | std::ofstream::badbit);
        EXPECT_CALL(*timestamp_producer_mock, getTimestamp()).Times(0);
        EXPECT_CALL(*timestamp_producer_mock, getTimestampInUs()).Times(0);

        EXPECT_NO_THROW(file_logs_producer.setupFile(kTestLogFileName, 0U, 0U));
    }

    TEST_F(FileLogsProducerTest, Setup_File_And_Log_File_Open_Successfully) {
        file_logs_producer.setupFile(kTestLogFileName, 0U, 0U);

        EXPECT_TRUE(file_logs_producer.GetLogFileStream().is_open());
    }

    TEST_F(FileLogsProducerTest, Try_Open_Log_File_Append_But_File_Is_Already_Opened) {
        file_logs_producer.setupFile(kTestLogFileName, 0U, 0U);
        EXPECT_CALL(*timestamp_producer_mock, getTimestamp()).Times(0);
        EXPECT_CALL(*timestamp_producer_mock, getTimestampInUs()).Times(0);

        EXPECT_NO_THROW(file_logs_producer.openLogFileAppend());
    }

    TEST_F(FileLogsProducerTest, Try_Open_Log_File_Append_But_Open_Failed) {
        file_logs_producer.GetLogFileStream().exceptions(std::ofstream::failbit | std::ofstream::badbit);
        EXPECT_CALL(*timestamp_producer_mock, getTimestamp()).Times(0);
        EXPECT_CALL(*timestamp_producer_mock, getTimestampInUs()).Times(0);

        EXPECT_NO_THROW(file_logs_producer.openLogFileAppend());
    }

    TEST_F(FileLogsProducerTest, Open_Log_File_Append_Successfully) {
        file_logs_producer.GetLogFileName() = kTestLogFileName;
        file_logs_producer.openLogFileAppend();

        EXPECT_TRUE(file_logs_producer.GetLogFileStream().is_open());
    }

    TEST_F(FileLogsProducerTest, Try_Open_Log_File_Truncate_But_File_Is_Already_Opened) {
        file_logs_producer.setupFile(kTestLogFileName, 0U, 0U);
        EXPECT_CALL(*timestamp_producer_mock, getTimestamp()).Times(0);
        EXPECT_CALL(*timestamp_producer_mock, getTimestampInUs()).Times(0);

        EXPECT_NO_THROW(file_logs_producer.openLogFileTruncate());
    }

    TEST_F(FileLogsProducerTest, Try_Open_Log_File_Truncate_But_Open_Failed) {
        file_logs_producer.GetLogFileStream().exceptions(std::ofstream::failbit | std::ofstream::badbit);
        EXPECT_CALL(*timestamp_producer_mock, getTimestamp()).Times(0);
        EXPECT_CALL(*timestamp_producer_mock, getTimestampInUs()).Times(0);

        EXPECT_NO_THROW(file_logs_producer.openLogFileTruncate());
    }

    TEST_F(FileLogsProducerTest, Open_Log_File_Truncate_Successfully) {
        file_logs_producer.GetLogFileName() = kTestLogFileName;
        file_logs_producer.openLogFileTruncate();

        EXPECT_TRUE(file_logs_producer.GetLogFileStream().is_open());
    }

    TEST_F(FileLogsProducerTest, Rotation_Is_Enabled_And_True_Returned) {
        file_logs_producer.GetMaxLogFileSizeBytes() = kTestMaxLogFileSizeBytes;
        file_logs_producer.GetMaxLogFiles() = kTestMaxLogFiles;

        EXPECT_TRUE(file_logs_producer.isRotationEnabled());
    }

    TEST_F(FileLogsProducerTest, Rotation_Is_Not_Enabled_And_False_Returned) {
        file_logs_producer.GetMaxLogFileSizeBytes() = 0U;
        file_logs_producer.GetMaxLogFiles() = kTestMaxLogFiles;

        EXPECT_FALSE(file_logs_producer.isRotationEnabled());

        file_logs_producer.GetMaxLogFileSizeBytes() = kTestMaxLogFileSizeBytes;
        file_logs_producer.GetMaxLogFiles() = 0U;

        EXPECT_FALSE(file_logs_producer.isRotationEnabled());
    }

    TEST_F(FileLogsProducerTest, Build_Rotated_File_Name_Correctly) {
        file_logs_producer.GetLogFileName() = kTestLogFileName;
        const std::string expectedFileName = "test_log_1.log";
        EXPECT_EQ(file_logs_producer.buildRotatedFileName(1U), expectedFileName);
    }

    TEST_F(FileLogsProducerTest, Build_Rotated_File_Name_Correctly_For_Higher_Index) {
        file_logs_producer.GetLogFileName() = kTestLogFileName;
        const std::string expectedFileName = "test_log_3.log";
        EXPECT_EQ(file_logs_producer.buildRotatedFileName(3U), expectedFileName);
    }

    TEST_F(FileLogsProducerTest, Try_Rotate_If_Needed_But_Rotation_Is_Not_Enabled) {
        file_logs_producer.GetMaxLogFileSizeBytes() = 0U;
        file_logs_producer.GetMaxLogFiles() = 0U;
        EXPECT_CALL(*timestamp_producer_mock, getTimestamp()).Times(0);
        EXPECT_CALL(*timestamp_producer_mock, getTimestampInUs()).Times(0);

        EXPECT_NO_THROW(file_logs_producer.rotateIfNeeded());
    }

    TEST_F(FileLogsProducerTest, Try_Rotate_If_Needed_But_File_Size_Is_Below_Threshold) {
        file_logs_producer.GetLogFileName() = kTestLogFileName;
        file_logs_producer.GetMaxLogFileSizeBytes() = 1024U;
        file_logs_producer.GetMaxLogFiles() = kTestMaxLogFiles;
        file_logs_producer.openLogFileAppend();
        EXPECT_CALL(*timestamp_producer_mock, getTimestamp()).Times(0);
        EXPECT_CALL(*timestamp_producer_mock, getTimestampInUs()).Times(0);

        EXPECT_NO_THROW(file_logs_producer.rotateIfNeeded());
        EXPECT_TRUE(file_logs_producer.GetLogFileStream().is_open());
    }

    TEST_F(FileLogsProducerTest, Try_Rotate_If_Needed_But_File_Is_Not_Open) {
        file_logs_producer.GetMaxLogFileSizeBytes() = kTestMaxLogFileSizeBytes;
        file_logs_producer.GetMaxLogFiles() = kTestMaxLogFiles;
        EXPECT_CALL(*timestamp_producer_mock, getTimestamp()).Times(0);
        EXPECT_CALL(*timestamp_producer_mock, getTimestampInUs()).Times(0);

        EXPECT_NO_THROW(file_logs_producer.rotateIfNeeded());
    }

    TEST_F(FileLogsProducerTest, Try_Rotate_But_Max_Log_Files_Is_Zero) {
        file_logs_producer.GetLogFileName() = kTestLogFileName;
        file_logs_producer.GetMaxLogFileSizeBytes() = 1U;
        file_logs_producer.GetMaxLogFiles() = 0U;
        file_logs_producer.openLogFileAppend();
        EXPECT_CALL(*timestamp_producer_mock, getTimestamp()).Times(0);
        EXPECT_CALL(*timestamp_producer_mock, getTimestampInUs()).Times(0);

        EXPECT_NO_THROW(file_logs_producer.rotateIfNeeded());
        EXPECT_TRUE(file_logs_producer.GetLogFileStream().is_open());
    }

    TEST_F(FileLogsProducerTest, Try_Rotate_If_Needed_But_Close_File_Failed) {
        file_logs_producer.GetLogFileName() = kTestLogFileName;
        file_logs_producer.GetMaxLogFileSizeBytes() = 1U;
        file_logs_producer.GetMaxLogFiles() = kTestMaxLogFiles;
        file_logs_producer.openLogFileAppend();
        file_logs_producer.GetLogFileStream().exceptions(std::ofstream::failbit | std::ofstream::badbit);
        EXPECT_CALL(*timestamp_producer_mock, getTimestamp()).Times(0);
        EXPECT_CALL(*timestamp_producer_mock, getTimestampInUs()).Times(0);

        EXPECT_NO_THROW(file_logs_producer.rotateIfNeeded());
    }

    TEST_F(FileLogsProducerTest, Rotate_If_Needed_And_Rotation_Performed) {
        file_logs_producer.GetLogFileName() = kTestLogFileName;
        file_logs_producer.GetMaxLogFileSizeBytes() = 1U;
        file_logs_producer.GetMaxLogFiles() = kTestMaxLogFiles;
        file_logs_producer.openLogFileAppend();
        EXPECT_CALL(*timestamp_producer_mock, getTimestamp()).Times(1).WillOnce(Return("time"));
        EXPECT_CALL(*timestamp_producer_mock, getTimestampInUs()).Times(1).WillOnce(Return("123"));
        file_logs_producer.logMessage("x");

        EXPECT_NO_THROW(file_logs_producer.rotateIfNeeded());
        EXPECT_TRUE(file_logs_producer.GetLogFileStream().is_open());
        EXPECT_EQ(file_logs_producer.GetNextRotationIndex(), 2U);
    }

    TEST_F(FileLogsProducerTest, Try_Log_Message_But_File_Is_Not_Open) {
        EXPECT_CALL(*timestamp_producer_mock, getTimestamp()).Times(0);
        EXPECT_CALL(*timestamp_producer_mock, getTimestampInUs()).Times(0);

        EXPECT_NO_THROW(file_logs_producer.logMessage("Test message"));
    }

    TEST_F(FileLogsProducerTest, Try_Log_Message_But_Write_Failed) {
        file_logs_producer.GetLogFileName() = kTestLogFileName;
        file_logs_producer.openLogFileAppend();
        file_logs_producer.GetLogFileStream().exceptions(std::ofstream::failbit | std::ofstream::badbit);
        EXPECT_CALL(*timestamp_producer_mock, getTimestamp()).Times(1);
        EXPECT_CALL(*timestamp_producer_mock, getTimestampInUs()).Times(1);

        EXPECT_NO_THROW(file_logs_producer.logMessage("Test message"));
    }

    TEST_F(FileLogsProducerTest, Log_Message_Successfully) {
        const std::string testMessage = "Test message";
        const std::string expectedLoggedMessage = "2024-06-01 12:00:00.000000" + testMessage;
        file_logs_producer.GetLogFileName() = kTestLogFileName;
        file_logs_producer.openLogFileTruncate();
        EXPECT_CALL(*timestamp_producer_mock, getTimestamp()).Times(1).WillOnce(Return("2024-06-01 12:00:00."));
        EXPECT_CALL(*timestamp_producer_mock, getTimestampInUs()).Times(1).WillOnce(Return("000000"));
        EXPECT_NO_THROW(file_logs_producer.logMessage(testMessage));
        file_logs_producer.GetLogFileStream().close();
        std::ifstream readFile(file_logs_producer.GetLogFileName());
        std::string loggedMessage;
        std::getline(readFile, loggedMessage);
        readFile.close();

        EXPECT_EQ(loggedMessage, expectedLoggedMessage);
    }

    TEST_F(FileLogsProducerTest, Try_Flush_But_File_Is_Not_Open) {
        EXPECT_CALL(*timestamp_producer_mock, getTimestamp()).Times(0);
        EXPECT_CALL(*timestamp_producer_mock, getTimestampInUs()).Times(0);

        EXPECT_NO_THROW(file_logs_producer.flush());
    }

    TEST_F(FileLogsProducerTest, Try_Flush_But_Flush_Failed) {
        file_logs_producer.GetLogFileName() = kTestLogFileName;
        file_logs_producer.openLogFileAppend();
        file_logs_producer.GetLogFileStream().exceptions(std::ofstream::failbit | std::ofstream::badbit);
        EXPECT_CALL(*timestamp_producer_mock, getTimestamp()).Times(0);
        EXPECT_CALL(*timestamp_producer_mock, getTimestampInUs()).Times(0);

        EXPECT_NO_THROW(file_logs_producer.flush());
    }

    TEST_F(FileLogsProducerTest, Flush_Successfully) {
        file_logs_producer.GetLogFileName() = kTestLogFileName;
        file_logs_producer.openLogFileAppend();
        EXPECT_CALL(*timestamp_producer_mock, getTimestamp()).Times(0);
        EXPECT_CALL(*timestamp_producer_mock, getTimestampInUs()).Times(0);

        EXPECT_NO_THROW(file_logs_producer.flush());
    }

}  // namespace file_logs_producer_test