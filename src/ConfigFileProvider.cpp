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
    }  // namespace

    ConfigFileProvider::ConfigFileProvider() {}

    std::optional<LoggerConfig> ConfigFileProvider::loadConfigFromFile(const std::string& configFilePath) {
        std::unordered_map<std::string, std::string> loadConfigMap = loadConfig(configFilePath);

        if (loadConfigMap.empty()) {
            throw std::runtime_error("Config file is empty or missing required keys: " + configFilePath);
            return std::nullopt;
        }

        LoggerConfig loggerConfig;
        loggerConfig.SetLogLevel(std::stoi(loadConfigMap.at("logLevel")));
        loggerConfig.logPrefix = loadConfigMap.at("logPrefix");
        loggerConfig.SetLogsOutputSink(std::stoi(loadConfigMap.at("logsOutputSink")));
        loggerConfig.logFileName = loadConfigMap.at("logFileName");
        loggerConfig.maxLogFileSizeBytes = std::stoll(loadConfigMap.at("maxLogFileSizeBytes"));
        loggerConfig.maxLogFiles = std::stoi(loadConfigMap.at("maxLogFiles"));

        return loggerConfig;
    }

    std::string ConfigFileProvider::trim(const std::string& string_to_trimmed) {
        const std::size_t first = string_to_trimmed.find_first_not_of(" \t\r\n");
        if (first == std::string::npos)
            return "";

        const std::size_t last = string_to_trimmed.find_last_not_of(" \t\r\n");

        return string_to_trimmed.substr(first, last - first + 1);
    }

    std::unordered_map<std::string, std::string> ConfigFileProvider::loadConfig(const std::string& file_path) {
        std::unordered_map<std::string, std::string> config;

        std::ifstream file(file_path);

        if (!file.is_open()) {
            throw std::runtime_error("Cannot open config file: " + file_path);
        }

        std::string line;

        while (std::getline(file, line)) {
            line = trim(line);

            if (line.empty()) {
                continue;
            }

            if (line[0] == kCommentChar) {
                continue;
            }

            const auto separator = line.find(kKeyValueSeparator);

            if (separator == std::string::npos) {
                continue;
            }

            std::string key = trim(line.substr(0, separator));
            std::string value = trim(line.substr(separator + 1));

            config[key] = value;
        }

        return config;
    }
}  // namespace equinox