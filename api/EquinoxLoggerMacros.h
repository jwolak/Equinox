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

#include "EquinoxLogger.h"

#if defined(NDEBUG)
#define LOG_TRACE(...) \
    do {               \
    } while (0)
#define LOG_DEBUG(...) \
    do {               \
    } while (0)
#else
#define LOG_TRACE(...) equinox::trace(__VA_ARGS__)
#define LOG_DEBUG(...) equinox::debug(__VA_ARGS__)
#endif

#define LOG_ERROR(...)    equinox::error(__VA_ARGS__)
#define LOG_WARNING(...)  equinox::warning(__VA_ARGS__)
#define LOG_INFO(...)     equinox::info(__VA_ARGS__)
#define LOG_CRITICAL(...) equinox::critical(__VA_ARGS__)

#define SETUP_LOGGER(logLevel, logPrefix, logsOutputSink, logFileName, maxLogFileSizeBytes, maxLogFiles) \
    equinox::setup(logLevel, logPrefix, logsOutputSink, logFileName, maxLogFileSizeBytes, maxLogFiles)

#define SETUP_FROM_CONFIG_FILE(configFilePath) equinox::setupFromConfigFile(configFilePath)

#define CHANGE_LOG_LEVEL(newLogLevel) equinox::changeLevel(newLogLevel)

#define CHANGE_LOGS_SINK(newLogsOutputSink) equinox::changeLogsSink(newLogsOutputSink)

#define FLUSH equinox::flush()