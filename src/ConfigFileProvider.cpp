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
#include <string>
#include <unordered_map>

#include "ConfigFileProvider.h"

namespace equinox {

    namespace {
        constexpr const char kCommentChar = '#';
        constexpr const char kKeyValueSeparator = '=';
        constexpr const char kWhitespaceChars[] = " \t\r\n";
    }  // namespace

    ConfigFileProvider::ConfigFileProvider() {}

    std::optional<LoggerConfig> ConfigFileProvider::loadConfigFromFile(const std::string& configFilePath) {
        try {
            std::unordered_map<std::string, std::string> loadConfigMap = loadConfig(configFilePath);

            if (loadConfigMap.empty()) {
                return std::nullopt;
            }

            LoggerConfig loggerConfig;
            loggerConfig.SetLogLevelFromInt(std::stoi(loadConfigMap.at("logLevel")));
            loggerConfig.logPrefix = loadConfigMap.at("logPrefix");
            loggerConfig.SetLogsOutputSinkFromInt(std::stoi(loadConfigMap.at("logsOutputSink")));
            loggerConfig.logFileName = loadConfigMap.at("logFileName");
            loggerConfig.maxLogFileSizeBytes = std::stoll(loadConfigMap.at("maxLogFileSizeBytes"));
            loggerConfig.maxLogFiles = std::stoi(loadConfigMap.at("maxLogFiles"));

            return loggerConfig;
        } catch (const std::exception&) {
            return std::nullopt;
        }
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