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

#include <cctype>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

#include "ConfigFileProvider.h"

namespace equinox {

    namespace {
        constexpr const char kCommentChar = '#';
        constexpr const char kKeyValueSeparator = '=';
        constexpr const char kWhitespaceChars[] = " \t\r\n";
        constexpr const char kLogLevelKey[] = "logLevel";
        constexpr const char kLogPrefixKey[] = "logPrefix";
        constexpr const char kLogsOutputSinkKey[] = "logsOutputSink";
        constexpr const char kLogFileNameKey[] = "logFileName";
        constexpr const char kMaxLogFileSizeBytesKey[] = "maxLogFileSizeBytes";
        constexpr const char kMaxLogFilesKey[] = "maxLogFiles";
    }  // namespace

    ConfigFileProvider::ConfigFileProvider() {}

    LoggerConfig ConfigFileProvider::loadConfigFromFile(const std::string& configFilePath) {
        std::unordered_map<std::string, std::string> loadConfigMap;
        LoggerConfig loggerConfig;
        loggerConfig.SetDefaults();

        try {
            loadConfigMap = loadConfig(configFilePath);
        } catch (const std::exception&) {
            return loggerConfig;
        }

        if (loadConfigMap.empty()) {
            return loggerConfig;
        }

        try {
            loggerConfig.SetLogLevelFromInt(std::stoi(loadConfigMap.at(kLogLevelKey)));
        } catch (const std::exception&) {
            std::cerr << "[EQUINOX LOGGER] [ERROR] Failed to parse log level from config file." << std::endl;
        }

        try {
            const std::string logPrefixValue = loadConfigMap.at(kLogPrefixKey);
            if (!logPrefixValue.empty()) {
                loggerConfig.logPrefix = logPrefixValue;
            }
        } catch (const std::exception&) {
            std::cerr << "[EQUINOX LOGGER] [ERROR] Failed to parse log prefix from config file." << std::endl;
        }

        try {
            loggerConfig.SetLogsOutputSinkFromInt(std::stoi(loadConfigMap.at(kLogsOutputSinkKey)));
        } catch (const std::exception&) {
            std::cerr << "[EQUINOX LOGGER] [ERROR] Failed to parse logs output sink from config file." << std::endl;
        }

        try {
            const std::string logFileNameValue = loadConfigMap.at(kLogFileNameKey);
            if (!logFileNameValue.empty()) {
                loggerConfig.logFileName = logFileNameValue;
            }
        } catch (const std::exception&) {
            std::cerr << "[EQUINOX LOGGER] [ERROR] Failed to parse log file name from config file." << std::endl;
        }
        try {
            loggerConfig.maxLogFileSizeBytes = std::stoll(loadConfigMap.at(kMaxLogFileSizeBytesKey));
        } catch (const std::exception&) {
            std::cerr << "[EQUINOX LOGGER] [ERROR] Failed to parse max log file size from config file." << std::endl;
        }

        try {
            const std::string maxLogFilesValue = loadConfigMap.at(kMaxLogFilesKey);
            if (!maxLogFilesValue.empty()) {
                loggerConfig.maxLogFiles = std::stoi(maxLogFilesValue);
            }
        } catch (const std::exception&) {
            std::cerr << "[EQUINOX LOGGER] [ERROR] Failed to parse max log files from config file." << std::endl;
        }

        return loggerConfig;
    }

    std::string ConfigFileProvider::trim(const std::string& string_to_trimmed) {
        const std::size_t first = string_to_trimmed.find_first_not_of(kWhitespaceChars);
        if (first == std::string::npos)
            return std::string();

        const std::size_t last = string_to_trimmed.find_last_not_of(kWhitespaceChars);

        return string_to_trimmed.substr(first, last - first + 1);
    }

    std::unordered_map<std::string, std::string> ConfigFileProvider::loadConfig(const std::string& file_path) {
        std::unordered_map<std::string, std::string> config;

        std::ifstream config_file(file_path);

        if (!config_file.is_open()) {
            throw std::runtime_error("Cannot open config file: " + file_path);
        }

        std::string read_line;

        while (std::getline(config_file, read_line)) {
            read_line = trim(read_line);

            if (read_line.empty()) {
                continue;
            }

            if (read_line[0] == kCommentChar) {
                continue;
            }

            std::size_t separator = read_line.find(kKeyValueSeparator);

            if (separator == std::string::npos) {
                continue;
            }

            std::string key = trim(read_line.substr(0, separator));
            std::string value = trim(read_line.substr(separator + 1));

            config[key] = value;
        }

        return config;
    }
}  // namespace equinox