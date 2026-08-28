/*
 * EquinoxLogger.h
 *
 *  Created on: 2023
 *      Author: Janusz Wolak
 */

/*-
 * BSD 3-Clause License
 *
 * Copyright (c) 2023, Janusz Wolak
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

#ifndef API_EQUINOXLOGGER_H_
#define API_EQUINOXLOGGER_H_

#include "EquinoxLoggerCommon.h"
#include "EquinoxLoggerEngine.h"

namespace equinox {

    /**
     * @brief trace() function to produce message with severity set to 'trace'
     *
     * @param format includes the message, or/and format specifier for the values included in the message
     * @param args variadic number of arguments to be logged
     */
    template <typename... Args>
    inline void trace(const std::string& format, Args&&... args) {
        equinox::EquinoxLoggerEngine::getInstance().log(level::LOG_LEVEL::trace, format, std::forward<Args>(args)...);
    }

    /**
     * @brief debug() function to produce message with severity set to 'debug'
     *
     * @param format includes the message, or/and format specifier for the values included in the message
     * @param args variadic number of arguments to be logged
     */
    template <typename... Args>
    inline void debug(const std::string& format, Args&&... args) {
        equinox::EquinoxLoggerEngine::getInstance().log(level::LOG_LEVEL::debug, format, std::forward<Args>(args)...);
    }

    /**
     * @brief info() function to produce message with severity set to 'info'
     *
     * @param format includes the message, or/and format specifier for the values included in the message
     * @param args variadic number of arguments to be logged
     */
    template <typename... Args>
    inline void info(const std::string& format, Args&&... args) {
        equinox::EquinoxLoggerEngine::getInstance().log(level::LOG_LEVEL::info, format, std::forward<Args>(args)...);
    }

    /**
     * @brief warning() function to produce message with severity set to 'warning'
     *
     * @param format includes the message, or/and format specifier for the values included in the message
     * @param args variadic number of arguments to be logged
     */
    template <typename... Args>
    inline void warning(const std::string& format, Args&&... args) {
        equinox::EquinoxLoggerEngine::getInstance().log(level::LOG_LEVEL::warning, format, std::forward<Args>(args)...);
    }

    /**
     * @brief error() function to produce message with severity set to 'error'
     *
     * @param format includes the message, or/and format specifier for the values included in the message
     * @param args variadic number of arguments to be logged
     */
    template <typename... Args>
    inline void error(const std::string& format, Args&&... args) {
        equinox::EquinoxLoggerEngine::getInstance().log(level::LOG_LEVEL::error, format, std::forward<Args>(args)...);
    }

    /**
     * @brief critical() function to produce message with severity set to 'critical'
     *
     * @param format includes the message, or/and format specifier for the values included in the message
     * @param args variadic number of arguments to be logged
     */
    template <typename... Args>
    inline void critical(const std::string& format, Args&&... args) {
        equinox::EquinoxLoggerEngine::getInstance().log(level::LOG_LEVEL::critical, format, std::forward<Args>(args)...);
    }

    /**
     * @brief setup() function to setup logger
     *
     * @param logLevel        level of the messages that will be (trace, debug, info, warning, error or critical)
     * @param logPrefix       string with prefix included to each log (f.ex. with application name)
     * @param logsOutputSink  place where logs are printed (console, file or both)
     * @param logFileName            name of file with logs (default: "logs.log")
     * @param maxLogFileSizeBytes    max size for single log file in bytes (default: 3 MB)
     * @param maxLogFiles            max number of rotated log files (default: 5)
     * @return true if setup succeeded, false if file setup failed
     */
    EQUINOX_API bool setup(level::LOG_LEVEL logLevel, const std::string& logPrefix, logs_output::SINK logsOutputSink,
                           const std::string& logFileName = kLogFileName, std::size_t maxLogFileSizeBytes = kDefaultMaxLogFileSizeBytes,
                           std::size_t maxLogFiles = kDefaultMaxLogFiles);

    /**
     * @brief setup() function to setup logger from configuration file
     *
     * @param configFilePath  path to the configuration file
     * @return true if setup succeeded, false if file setup failed
     */
    EQUINOX_API bool setupFromConfigFile(const std::string& configFilePath);

    /**
     * @brief changeLevel() function to change level of logged messages
     *
     * @param logLevel  level of the messages that will be (trace, debug, info, warning, error or critical)
     */
    EQUINOX_API void changeLevel(level::LOG_LEVEL logLevel);

    /**
     * @brief changeLogsOutputSink() function to change logs output
     *
     * @param logsOutputSink  place where logs are printed (console, file or both)
     * @return true if change succeeded, false if file setup failed
     */
    EQUINOX_API bool changeLogsOutputSink(logs_output::SINK logsOutputSink);

    /**
     * @brief flush() function to force write any pending log messages
     */
    EQUINOX_API void flush();

} /*namespace equinox*/

#endif /* API_EQUINOXLOGGER_H_ */
