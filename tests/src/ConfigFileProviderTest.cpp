/*-
 * BSD 3-Clause License
 *
 * Copyright (c) 2026, Janusz Wolak
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <cstdio>

#include <gtest/gtest.h>

#include <fstream>
#include <string>

#include "ConfigFileProvider.h"

namespace config_file_provider_test {
    using namespace ::testing;
    using namespace equinox;

    namespace {
        constexpr const char* kEmptyConfigFilePath = "empty_config_file.txt";
        constexpr const char* kNonExistentConfigFilePath = "non_existent_config_file.txt";
        constexpr const char* kConfigFileWithNoLogLevelPath = "config_file_with_no_log_level.txt";
        constexpr const char* kConfigFileWithNoLogPrefixPath = "config_file_with_no_log_prefix.txt";

        constexpr const char kLogLevelKey[] = "logLevel";
        constexpr const char kLogPrefixKey[] = "logPrefix";
        constexpr const char kLogsOutputSinkKey[] = "logsOutputSink";
        constexpr const char kLogFileNameKey[] = "logFileName";
        constexpr const char kMaxLogFileSizeBytesKey[] = "maxLogFileSizeBytes";
        constexpr const char kMaxLogFilesKey[] = "maxLogFiles";
    }  // namespace

    class ConfigFileProviderTestable : public ConfigFileProvider {
       public:
        ConfigFileProviderTestable() : ConfigFileProvider() {}

        using ConfigFileProvider::loadConfig;
        using ConfigFileProvider::trim;
    };

    class ConfigFileProviderTest : public Test {
       public:
        ConfigFileProviderTest() : default_logger_config{} {}

        ConfigFileProviderTestable config_file_provider;

        void CreateEmptyConfigFile(const std::string& file_path) {
            std::ofstream file(file_path);
        }

        void RemoveConfigFile(const std::string& file_path) {
            std::remove(file_path.c_str());
        }

        void CreateConfigFileWithProvidedContent(const std::string& file_path, const std::string& content) {
            std::ofstream file(file_path);
            file << content;
        }

        const LoggerConfig default_logger_config;
    };

    TEST_F(ConfigFileProviderTest, Try_Load_Config_From_File_But_Load_Config_Map_An_Exception_Thrown_And_Default_Settings_Returned) {
        EXPECT_EQ(config_file_provider.loadConfigFromFile(kNonExistentConfigFilePath), default_logger_config);
    }

    TEST_F(ConfigFileProviderTest, Try_Load_Config_From_File_But_Loaded_Config_Map_Is_Empty_And_Default_Settings_Returned) {
        CreateEmptyConfigFile(kEmptyConfigFilePath);

        EXPECT_EQ(config_file_provider.loadConfigFromFile(kEmptyConfigFilePath), default_logger_config);

        RemoveConfigFile(kEmptyConfigFilePath);
    }

    TEST_F(ConfigFileProviderTest, Try_Load_Config_From_File_But_File_Is_Empty_And_Defaults_Settings_Returned) {
        CreateEmptyConfigFile(kEmptyConfigFilePath);

        EXPECT_EQ(config_file_provider.loadConfigFromFile(kEmptyConfigFilePath), default_logger_config);

        RemoveConfigFile(kEmptyConfigFilePath);
    }

    TEST_F(ConfigFileProviderTest, Try_Read_Log_Level_From_Config_File_But_It_Fails_And_Default_Log_Level_Is_Set) {
        CreateConfigFileWithProvidedContent(kConfigFileWithNoLogLevelPath, std::string(kLogLevelKey) + "=\n");

        LoggerConfig logger_config = config_file_provider.loadConfigFromFile(kConfigFileWithNoLogLevelPath);
        EXPECT_EQ(logger_config.logLevel, default_logger_config.logLevel);

        RemoveConfigFile(kConfigFileWithNoLogLevelPath);
    }

    TEST_F(ConfigFileProviderTest, Read_Log_Level_Debug_From_Config_File_And_Log_Level_Debug_Is_Set) {
        CreateConfigFileWithProvidedContent(kConfigFileWithNoLogLevelPath, std::string(kLogLevelKey) + "= 1\n");

        LoggerConfig logger_config = config_file_provider.loadConfigFromFile(kConfigFileWithNoLogLevelPath);
        EXPECT_EQ(logger_config.logLevel, level::LOG_LEVEL::debug);

        RemoveConfigFile(kConfigFileWithNoLogLevelPath);
    }

    TEST_F(ConfigFileProviderTest, Try_Read_Log_Prefix_From_Config_File_But_It_Fails_And_Default_Log_Prefix_Is_Set) {
        CreateConfigFileWithProvidedContent(kConfigFileWithNoLogPrefixPath, std::string(kLogLevelKey) + "=\n");

        LoggerConfig logger_config = config_file_provider.loadConfigFromFile(kConfigFileWithNoLogPrefixPath);
        EXPECT_EQ(logger_config.logPrefix, kLogDefaultPrefix);

        RemoveConfigFile(kConfigFileWithNoLogPrefixPath);
    }

    TEST_F(ConfigFileProviderTest, Try_Read_Log_Prefix_From_Config_File_But_Is_Empty_And_Default_Log_Prefix_Is_Set) {
        CreateConfigFileWithProvidedContent(kConfigFileWithNoLogPrefixPath, std::string(kLogLevelKey) + "= 1\n" + std::string(kLogPrefixKey) + "=\n");

        LoggerConfig logger_config = config_file_provider.loadConfigFromFile(kConfigFileWithNoLogPrefixPath);
        EXPECT_EQ(logger_config.logPrefix, kLogDefaultPrefix);

        RemoveConfigFile(kConfigFileWithNoLogPrefixPath);
    }

    TEST_F(ConfigFileProviderTest, Read_Log_Prefix_From_Config_File_And_Log_Prefix_Is_Set) {
        CreateConfigFileWithProvidedContent(kConfigFileWithNoLogPrefixPath, std::string(kLogLevelKey) + "= 1\n" + std::string(kLogPrefixKey) + "= prefix\n");

        LoggerConfig logger_config = config_file_provider.loadConfigFromFile(kConfigFileWithNoLogPrefixPath);
        EXPECT_EQ(logger_config.logPrefix, "prefix");

        RemoveConfigFile(kConfigFileWithNoLogPrefixPath);
    }

    TEST_F(ConfigFileProviderTest, Try_Read_Logs_Output_Sink_From_Config_File_But_It_Fails_And_Default_Logs_Output_Sink_Is_Set) {
        CreateConfigFileWithProvidedContent(kConfigFileWithNoLogPrefixPath, std::string(kLogLevelKey) + "= 1\n" + std::string(kLogPrefixKey) + "= prefix\n");

        LoggerConfig logger_config = config_file_provider.loadConfigFromFile(kConfigFileWithNoLogPrefixPath);
        EXPECT_EQ(logger_config.logsOutputSink, logs_output::SINK::file);

        RemoveConfigFile(kConfigFileWithNoLogPrefixPath);
    }

    TEST_F(ConfigFileProviderTest, Try_Read_Logs_Output_Sink_From_Config_File_But_Is_Empty_And_Default_Logs_Output_Sink_Is_Set) {
        CreateConfigFileWithProvidedContent(
            kConfigFileWithNoLogPrefixPath,
            std::string(kLogLevelKey) + "= 1\n" + std::string(kLogPrefixKey) + "= prefix\n" + std::string(kLogsOutputSinkKey) + "=\n");

        LoggerConfig logger_config = config_file_provider.loadConfigFromFile(kConfigFileWithNoLogPrefixPath);
        EXPECT_EQ(logger_config.logsOutputSink, logs_output::SINK::file);

        RemoveConfigFile(kConfigFileWithNoLogPrefixPath);
    }

    TEST_F(ConfigFileProviderTest, Read_Logs_Output_Sink_From_Config_File_And_Logs_Output_Sink_Is_Set) {
        CreateConfigFileWithProvidedContent(
            kConfigFileWithNoLogPrefixPath,
            std::string(kLogLevelKey) + "= 1\n" + std::string(kLogPrefixKey) + "= prefix\n" + std::string(kLogsOutputSinkKey) + "= 0\n");

        LoggerConfig logger_config = config_file_provider.loadConfigFromFile(kConfigFileWithNoLogPrefixPath);
        EXPECT_EQ(logger_config.logsOutputSink, logs_output::SINK::console);

        RemoveConfigFile(kConfigFileWithNoLogPrefixPath);
    }

    TEST_F(ConfigFileProviderTest, Try_Read_Log_File_Name_From_Config_File_But_It_Fails_And_Default_Log_File_Name_Is_Set) {}

    TEST_F(ConfigFileProviderTest, Try_Read_Max_Log_File_Size_Bytes_From_Config_File_But_It_Fails_And_Default_Max_Log_File_Size_Bytes_Is_Set) {}

    TEST_F(ConfigFileProviderTest, Try_Read_Max_Log_Files_From_Config_File_But_It_Fails_And_Default_Max_Log_Files_Is_Set) {}

    TEST_F(ConfigFileProviderTest, Load_Config_From_File_Successfull_And_Configuration_Returned) {}

    TEST_F(ConfigFileProviderTest, Trim_Empty_String_And_Empty_String_Returned) {}

    TEST_F(ConfigFileProviderTest, Trim_String_With_Leading_And_Trailing_Whitespace_And_Trimmed_String_Returned) {}

    TEST_F(ConfigFileProviderTest, Trim_String_With_No_Leading_Or_Trailing_Whitespace_And_Original_String_Returned) {}

    TEST_F(ConfigFileProviderTest, Trim_String_With_Only_Whitespace_And_Empty_String_Returned) {}

    TEST_F(ConfigFileProviderTest, Trim_String_With_Leading_Whitespace_Only_And_Trimmed_String_Returned) {}

    TEST_F(ConfigFileProviderTest, Trim_String_With_Trailing_Whitespace_Only_And_Trimmed_String_Returned) {}

    TEST_F(ConfigFileProviderTest, Trim_String_With_Internal_Whitespace_And_Original_String_Returned) {}

    TEST_F(ConfigFileProviderTest, Trim_String_With_Mixed_Whitespace_And_Trimmed_String_Returned) {}

    TEST_F(ConfigFileProviderTest, Trim_String_With_No_Whitespace_And_Original_String_Returned) {}

    TEST_F(ConfigFileProviderTest, Trim_String_With_Leading_And_Internal_Whitespace_And_Trimmed_String_Returned) {}

    TEST_F(ConfigFileProviderTest, Trim_String_With_Trailing_And_Internal_Whitespace_And_Trimmed_String_Returned) {}

    TEST_F(ConfigFileProviderTest, Trim_String_With_Leading_Trailing_And_Internal_Whitespace_And_Trimmed_String_Returned) {}

    TEST_F(ConfigFileProviderTest, Trim_String_With_Leading_And_Trailing_Whitespace_Only_And_Trimmed_String_Returned) {}

    TEST_F(ConfigFileProviderTest, Try_Load_Config_From_File_But_Cannot_Open_File_And_Exception_Thrown) {}

    TEST_F(ConfigFileProviderTest, Try_Load_Config_From_File_But_File_Does_Not_Exist_And_Exception_Thrown) {}

    TEST_F(ConfigFileProviderTest, Try_Load_Config_From_File_But_File_Is_Empty_And_Exception_Thrown) {}

    TEST_F(ConfigFileProviderTest, Try_Load_Config_From_File_But_File_Has_Invalid_Format_And_Exception_Thrown) {}

    TEST_F(ConfigFileProviderTest, Try_Load_Config_From_File_But_File_Has_Missing_Required_Fields_And_Exception_Thrown) {}

    TEST_F(ConfigFileProviderTest, Try_Load_Config_From_File_But_File_Has_Extra_Unexpected_Fields_And_Exception_Thrown) {}

    TEST_F(ConfigFileProviderTest, Try_Load_Config_From_File_But_File_Has_Permission_Issues_And_Exception_Thrown) {}

    TEST_F(ConfigFileProviderTest, Try_Load_Config_From_File_But_File_Has_Symbolic_Link_And_Exception_Thrown) {}

    TEST_F(ConfigFileProviderTest, Try_Load_Config_From_File_But_File_Has_Circular_Symbolic_Link_And_Exception_Thrown) {}

    TEST_F(ConfigFileProviderTest, Try_Load_Config_From_File_But_File_Has_Unsupported_Encoding_And_Exception_Thrown) {}

    TEST_F(ConfigFileProviderTest, Load_Config_From_File_With_Empty_Lines_And_Configuration_Returned) {}

    TEST_F(ConfigFileProviderTest, Load_Config_From_File_With_Commented_Lines_And_Configuration_Returned) {}

    TEST_F(ConfigFileProviderTest, Load_Config_From_File_With_Separator_At_Start_Of_Line_And_Configuration_Returned) {}

    TEST_F(ConfigFileProviderTest, Load_Config_From_File_With_Valid_Config_And_Configuration_Returned) {}

}  // namespace config_file_provider_test