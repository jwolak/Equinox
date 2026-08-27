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

#pragma once

#include <cstddef>
#include <cstdint>

#include <string>

#include "EquinoxLoggerCommon.h"

namespace equinox {
    struct LoggerConfig {
        level::LOG_LEVEL logLevel = level::LOG_LEVEL::info;
        std::string logPrefix;
        logs_output::SINK logsOutputSink = logs_output::SINK::console;
        std::string logFileName = kLogFileName;
        std::size_t maxLogFileSizeBytes = kDefaultMaxLogFileSizeBytes;
        std::size_t maxLogFiles = kDefaultMaxLogFiles;

        void SetLogLevel(int32_t logLevel) {
            switch (logLevel) {
                case 0:
                    this->logLevel = level::LOG_LEVEL::trace;
                    break;

                case 1:
                    this->logLevel = level::LOG_LEVEL::debug;
                    break;

                case 2:
                    this->logLevel = level::LOG_LEVEL::info;
                    break;

                case 3:
                    this->logLevel = level::LOG_LEVEL::warning;
                    break;

                case 4:
                    this->logLevel = level::LOG_LEVEL::error;
                    break;

                case 5:
                    this->logLevel = level::LOG_LEVEL::critical;
                    break;
            }
        }

        void SetLogsOutputSink(int32_t logsOutputSink) {
            switch (logsOutputSink) {
                case 0:
                    this->logsOutputSink = logs_output::SINK::console;
                    break;

                case 1:
                    this->logsOutputSink = logs_output::SINK::file;
                    break;

                case 2:
                    this->logsOutputSink = logs_output::SINK::console_and_file;
                    break;
            }
        }
    };
}  // namespace equinox