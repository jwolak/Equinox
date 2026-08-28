/*
 * EquinoxLogger.cpp
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

#include "EquinoxLogger.h"

bool equinox::setup(equinox::level::LOG_LEVEL logLevel, const std::string& logPrefix, equinox::logs_output::SINK logsOutputSink, const std::string& logFileName,
                    std::size_t maxLogFileSizeBytes, std::size_t maxLogFiles) {
    return equinox::EquinoxLoggerEngine::getInstance().setup(logLevel, logPrefix, logsOutputSink, logFileName, maxLogFileSizeBytes, maxLogFiles);
}

bool equinox::setupFromConfigFile(const std::string& configFilePath) {
    return equinox::EquinoxLoggerEngine::getInstance().setupFromConfigFile(configFilePath);
}

void equinox::changeLevel(equinox::level::LOG_LEVEL logLevel) {
    equinox::EquinoxLoggerEngine::getInstance().changeLevel(logLevel);
}

bool equinox::changeLogsOutputSink(logs_output::SINK logsOutputSink) {
    return equinox::EquinoxLoggerEngine::getInstance().changeLogsOutputSink(logsOutputSink);
}

void equinox::flush() {
    equinox::EquinoxLoggerEngine::getInstance().flush();
}
