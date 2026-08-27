/*
 * EquinoxLoggerEngineImpl.cpp
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

#include <fmt/format.h>

#include "EquinoxLoggerEngineImpl.h"

equinox::EquinoxLoggerEngineImpl::EquinoxLoggerEngineImpl()
    : mLoggerConfig_{},
      mTimestampProducer_{std::make_shared<TimestampProducer>()},
      mFileLogsProducer_{std::make_shared<FileLogsProducer>(mTimestampProducer_)},
      mAsyncLogQueueEngine_{std::make_unique<AsyncLogQueueEngine>(mTimestampProducer_, mFileLogsProducer_, logs_output::SINK::console)},
      mConfigFileProvider_{std::make_unique<ConfigFileProvider>()} {}

equinox::EquinoxLoggerEngineImpl::EquinoxLoggerEngineImpl(std::shared_ptr<ITimestampProducer> mTimestampProducer,
                                                          std::shared_ptr<IFileLogsProducer> mFileLogsProducer,
                                                          std::unique_ptr<IAsyncLogQueueEngine> mAsyncLogQueueEngine,
                                                          std::unique_ptr<IConfigFileProvider> mConfigFileProvider)
    : mLoggerConfig_{},
      mTimestampProducer_{mTimestampProducer},
      mFileLogsProducer_{mFileLogsProducer},
      mAsyncLogQueueEngine_{std::move(mAsyncLogQueueEngine)},
      mConfigFileProvider_{std::move(mConfigFileProvider)} {}

const std::string& equinox::EquinoxLoggerEngineImpl::getLogPrefix() const {
    return mLoggerConfig_.logPrefix;
}

equinox::level::LOG_LEVEL equinox::EquinoxLoggerEngineImpl::getLogLevel() const {
    return mLoggerConfig_.logLevel;
}

bool equinox::EquinoxLoggerEngineImpl::shouldLog(level::LOG_LEVEL msgLevel) const {
    if (msgLevel == level::LOG_LEVEL::off) {
        return false;
    }

    return msgLevel >= mLoggerConfig_.logLevel;
}

const std::string& equinox::EquinoxLoggerEngineImpl::getLogFileName() const {
    return mLoggerConfig_.logFileName;
}

std::size_t equinox::EquinoxLoggerEngineImpl::getMaxLogFileSizeBytes() const {
    return mLoggerConfig_.maxLogFileSizeBytes;
}

std::size_t equinox::EquinoxLoggerEngineImpl::getMaxLogFiles() const {
    return mLoggerConfig_.maxLogFiles;
}

void equinox::EquinoxLoggerEngineImpl::logMessage(level::LOG_LEVEL msgLevel, const std::string& formatedOutputMessage) {
    if (!shouldLog(msgLevel)) {
        return;
    }

    thread_local std::string outputMessage;
    outputMessage.clear();

    switch (msgLevel) {
        case level::LOG_LEVEL::critical:
            outputMessage = fmt::format("{}[CRITICAL] {}", mLoggerConfig_.logPrefix, formatedOutputMessage);
            break;

        case level::LOG_LEVEL::debug:
            outputMessage = fmt::format("{}[DEBUG] {}", mLoggerConfig_.logPrefix, formatedOutputMessage);
            break;

        case level::LOG_LEVEL::error:
            outputMessage = fmt::format("{}[ERROR] {}", mLoggerConfig_.logPrefix, formatedOutputMessage);
            break;

        case level::LOG_LEVEL::info:
            outputMessage = fmt::format("{}[INFO] {}", mLoggerConfig_.logPrefix, formatedOutputMessage);
            break;

        case level::LOG_LEVEL::trace:
            outputMessage = fmt::format("{}[TRACE] {}", mLoggerConfig_.logPrefix, formatedOutputMessage);
            break;

        case level::LOG_LEVEL::warning:
            outputMessage = fmt::format("{}[WARNING] {}", mLoggerConfig_.logPrefix, formatedOutputMessage);
            break;

        case level::LOG_LEVEL::off:
            // Should not reach here due to the check above, but included for completeness
            break;
    }

    mAsyncLogQueueEngine_->startWorkerIfNeeded();
    mAsyncLogQueueEngine_->processLogMessage(outputMessage);
}

bool equinox::EquinoxLoggerEngineImpl::setup(level::LOG_LEVEL logLevel, const std::string& logPrefix, logs_output::SINK logsOutputSink,
                                             const std::string& logFileName, std::size_t maxLogFileSizeBytes, std::size_t maxLogFiles) {
    mAsyncLogQueueEngine_->stopWorker();

    mLoggerConfig_.logLevel = logLevel;
    mLoggerConfig_.logPrefix = std::string("[" + logPrefix + "]");
    mAsyncLogQueueEngine_->setLogsOutputSink(logsOutputSink);
    mLoggerConfig_.logFileName = logFileName;
    mLoggerConfig_.maxLogFileSizeBytes = maxLogFileSizeBytes;
    mLoggerConfig_.maxLogFiles = maxLogFiles;

    if (equinox::logs_output::SINK::file == logsOutputSink or equinox::logs_output::SINK::console_and_file == logsOutputSink) {
        try {
            mFileLogsProducer_->setupFile(mLoggerConfig_.logFileName, mLoggerConfig_.maxLogFileSizeBytes, mLoggerConfig_.maxLogFiles);
        } catch (const std::exception& ex) {
            std::cerr << fmt::format("[EquinoxLogger] Failed to setup log file: {}", ex.what()) << std::endl;
            return false;
        }
    }

    mAsyncLogQueueEngine_->startWorkerIfNeeded();
    return true;
}

bool equinox::EquinoxLoggerEngineImpl::setupFromConfigFile(const std::string& configFilePath) {
    mAsyncLogQueueEngine_->stopWorker();

    if (configFilePath.empty()) {
        std::cerr << "[EquinoxLogger] Configuration file path is empty. Default settings applied." << std::endl;
        return false;
    }

    std::optional<LoggerConfig> logger_config {LoggerConfig{}};
    logger_config->SetDefaults();

    logger_config = mConfigFileProvider_->loadConfigFromFile(configFilePath);
    if (logger_config.has_value()) {
        mLoggerConfig_ = logger_config.value();
    } else {
        std::cerr << fmt::format("[EquinoxLogger] Failed to load configuration from file: {}. Default settings applied.", configFilePath) << std::endl;
        return false;
    }

    if (equinox::logs_output::SINK::file == mLoggerConfig_.logsOutputSink or equinox::logs_output::SINK::console_and_file == mLoggerConfig_.logsOutputSink) {
        try {
            mFileLogsProducer_->setupFile(mLoggerConfig_.logFileName, mLoggerConfig_.maxLogFileSizeBytes, mLoggerConfig_.maxLogFiles);
        } catch (const std::exception& ex) {
            std::cerr << fmt::format("[EquinoxLogger] Failed to setup log file: {}", ex.what()) << std::endl;
            return false;
        }
    }

    mAsyncLogQueueEngine_->startWorkerIfNeeded();
    return true;
}

void equinox::EquinoxLoggerEngineImpl::changeLevel(level::LOG_LEVEL logLevel) {
    mLoggerConfig_.logLevel = logLevel;
}

bool equinox::EquinoxLoggerEngineImpl::changeLogsOutputSink(logs_output::SINK logsOutputSink) {
    mAsyncLogQueueEngine_->stopWorker();
    mAsyncLogQueueEngine_->setLogsOutputSink(logsOutputSink);

    if (equinox::logs_output::SINK::file == logsOutputSink or equinox::logs_output::SINK::console_and_file == logsOutputSink) {
        try {
            mFileLogsProducer_->setupFile(mLoggerConfig_.logFileName, mLoggerConfig_.maxLogFileSizeBytes, mLoggerConfig_.maxLogFiles);
        } catch (const std::exception& ex) {
            std::cerr << fmt::format("[EquinoxLogger] Failed to switch to file output: {}", ex.what()) << std::endl;
            return false;
        }
    }

    mAsyncLogQueueEngine_->startWorkerIfNeeded();
    return true;
}

void equinox::EquinoxLoggerEngineImpl::flush() {
    mAsyncLogQueueEngine_->flush();
}
