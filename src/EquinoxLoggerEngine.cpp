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

#include "EquinoxLoggerEngine.h"
#include "EquinoxLoggerEngineImpl.h"

equinox::EquinoxLoggerEngine::EquinoxLoggerEngine() : mEquinoxLoggerEngineImpl_{std::make_unique<EquinoxLoggerEngineImpl>()} {}

equinox::EquinoxLoggerEngine::EquinoxLoggerEngine(std::unique_ptr<IEquinoxLoggerEngineImpl> mEquinoxLoggerEngineImpl)
    : mEquinoxLoggerEngineImpl_{std::move(mEquinoxLoggerEngineImpl)} {}

equinox::EquinoxLoggerEngine& equinox::EquinoxLoggerEngine::getInstance() {
    static EquinoxLoggerEngine sEquinoxLoggerEngine;
    return sEquinoxLoggerEngine;
}

bool equinox::EquinoxLoggerEngine::setup(equinox::level::LOG_LEVEL logLevel, const std::string& logPrefix, equinox::logs_output::SINK logsOutputSink,
                                         const std::string& logFileName, std::size_t maxLogFileSizeBytes, std::size_t maxLogFiles) {
    std::lock_guard<std::mutex> lock(mEngineMutex_);
    return mEquinoxLoggerEngineImpl_->setup(logLevel, logPrefix, logsOutputSink, logFileName, maxLogFileSizeBytes, maxLogFiles);
}

bool equinox::EquinoxLoggerEngine::setupFromConfigFile(const std::string& configFilePath) {
    std::lock_guard<std::mutex> lock(mEngineMutex_);
    return mEquinoxLoggerEngineImpl_->setupFromConfigFile(configFilePath);
}

void equinox::EquinoxLoggerEngine::changeLevel(level::LOG_LEVEL logLevel) {
    std::lock_guard<std::mutex> lock(mEngineMutex_);
    mEquinoxLoggerEngineImpl_->changeLevel(logLevel);
}

bool equinox::EquinoxLoggerEngine::changeLogsOutputSink(logs_output::SINK logsOutputSink) {
    std::lock_guard<std::mutex> lock(mEngineMutex_);
    return mEquinoxLoggerEngineImpl_->changeLogsOutputSink(logsOutputSink);
}
void equinox::EquinoxLoggerEngine::flush() {
    std::lock_guard<std::mutex> lock(mEngineMutex_);
    mEquinoxLoggerEngineImpl_->flush();
}