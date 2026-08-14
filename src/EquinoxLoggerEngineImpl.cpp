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
    : mLogPrefix_{},
      mLogLevel_{},
      mLogFileName_{},
      mMaxLogFileSizeBytes_{kDefaultMaxLogFileSizeBytes},
      mMaxLogFiles_{kDefaultMaxLogFiles},
      mTimestampProducer_{std::make_shared<TimestampProducer>()},
      mFileLogsProducer_{std::make_shared<FileLogsProducer>(mTimestampProducer_)},
      mAsyncLogQueueEngine_{std::make_unique<AsyncLogQueueEngine>(mTimestampProducer_, mFileLogsProducer_, logs_output::SINK::console)} {}

equinox::EquinoxLoggerEngineImpl::EquinoxLoggerEngineImpl(std::shared_ptr<ITimestampProducer> mTimestampProducer,
                                                          std::shared_ptr<IFileLogsProducer> mFileLogsProducer,
                                                          std::unique_ptr<IAsyncLogQueueEngine> mAsyncLogQueueEngine)
    : mLogPrefix_{},
      mLogLevel_{},
      mLogFileName_{},
      mMaxLogFileSizeBytes_{kDefaultMaxLogFileSizeBytes},
      mMaxLogFiles_{kDefaultMaxLogFiles},
      mTimestampProducer_{mTimestampProducer},
      mFileLogsProducer_{mFileLogsProducer},
      mAsyncLogQueueEngine_{std::move(mAsyncLogQueueEngine)} {}

const std::string& equinox::EquinoxLoggerEngineImpl::getLogPrefix() const {
    return mLogPrefix_;
}

equinox::level::LOG_LEVEL equinox::EquinoxLoggerEngineImpl::getLogLevel() const {
    return mLogLevel_;
}

bool equinox::EquinoxLoggerEngineImpl::shouldLog(level::LOG_LEVEL msgLevel) const {
    if (msgLevel == level::LOG_LEVEL::off) {
        return false;
    }

    return msgLevel >= mLogLevel_;
}

const std::string& equinox::EquinoxLoggerEngineImpl::getLogFileName() const {
    return mLogFileName_;
}

std::size_t equinox::EquinoxLoggerEngineImpl::getMaxLogFileSizeBytes() const {
    return mMaxLogFileSizeBytes_;
}

std::size_t equinox::EquinoxLoggerEngineImpl::getMaxLogFiles() const {
    return mMaxLogFiles_;
}

void equinox::EquinoxLoggerEngineImpl::logMessage(level::LOG_LEVEL msgLevel, const std::string& formatedOutputMessage) {
    if (!shouldLog(msgLevel)) {
        return;
    }

    thread_local std::string outputMessage;
    outputMessage.clear();

    switch (msgLevel) {
        case level::LOG_LEVEL::critical:
            outputMessage = fmt::format("{}[CRITICAL] {}", mLogPrefix_, formatedOutputMessage);
            break;

        case level::LOG_LEVEL::debug:
            outputMessage = fmt::format("{}[DEBUG] {}", mLogPrefix_, formatedOutputMessage);
            break;

        case level::LOG_LEVEL::error:
            outputMessage = fmt::format("{}[ERROR] {}", mLogPrefix_, formatedOutputMessage);
            break;

        case level::LOG_LEVEL::info:
            outputMessage = fmt::format("{}[INFO] {}", mLogPrefix_, formatedOutputMessage);
            break;

        case level::LOG_LEVEL::trace:
            outputMessage = fmt::format("{}[TRACE] {}", mLogPrefix_, formatedOutputMessage);
            break;

        case level::LOG_LEVEL::warning:
            outputMessage = fmt::format("{}[WARNING] {}", mLogPrefix_, formatedOutputMessage);
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

    mLogLevel_ = logLevel;
    mLogPrefix_ = std::string("[" + logPrefix + "]");
    mAsyncLogQueueEngine_->setLogsOutputSink(logsOutputSink);
    mLogFileName_ = logFileName;
    mMaxLogFileSizeBytes_ = maxLogFileSizeBytes;
    mMaxLogFiles_ = maxLogFiles;

    if (equinox::logs_output::SINK::file == logsOutputSink or equinox::logs_output::SINK::console_and_file == logsOutputSink) {
        try {
            mFileLogsProducer_->setupFile(mLogFileName_, mMaxLogFileSizeBytes_, mMaxLogFiles_);
        } catch (const std::exception& ex) {
            std::cerr << fmt::format("[EquinoxLogger] Failed to setup log file: {}", ex.what()) << std::endl;
            return false;
        }
    }

    mAsyncLogQueueEngine_->startWorkerIfNeeded();
    return true;
}

void equinox::EquinoxLoggerEngineImpl::changeLevel(level::LOG_LEVEL logLevel) {
    mLogLevel_ = logLevel;
}

bool equinox::EquinoxLoggerEngineImpl::changeLogsOutputSink(logs_output::SINK logsOutputSink) {
    mAsyncLogQueueEngine_->stopWorker();
    mAsyncLogQueueEngine_->setLogsOutputSink(logsOutputSink);

    if (equinox::logs_output::SINK::file == logsOutputSink or equinox::logs_output::SINK::console_and_file == logsOutputSink) {
        try {
            mFileLogsProducer_->setupFile(mLogFileName_, mMaxLogFileSizeBytes_, mMaxLogFiles_);
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
