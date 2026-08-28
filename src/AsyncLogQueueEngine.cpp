/*
 * AsyncLogQueueEngine.cpp
 *
 *  Created on: 2026
 *      Author: Janusz Wolak
 */

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

#include "AsyncLogQueueEngine.h"

namespace {
    static constexpr std::size_t kDefaultQueueMaxSize = 10000U;
    static constexpr std::size_t kDefaultBatchSize = 64U;
    static constexpr uint32_t kDefaultDequeueTimeoutMs = 50U;
}  // namespace

equinox::AsyncLogQueueEngine::AsyncLogQueueEngine(std::shared_ptr<ITimestampProducer> timestamp_procducer, std::shared_ptr<IFileLogsProducer> fileLogsProducer,
                                                  logs_output::SINK logsOutputSink)
    : AsyncLogQueueEngine(timestamp_procducer, std::make_unique<ConsoleLogsProducer>(timestamp_procducer), fileLogsProducer, logsOutputSink,
                          std::make_unique<AsyncLogQueue>(kDefaultQueueMaxSize)) {}

/* For tests purpose */
equinox::AsyncLogQueueEngine::AsyncLogQueueEngine(std::shared_ptr<ITimestampProducer> timestamp_procducer,
                                                  std::unique_ptr<IConsoleLogsProducer> consoleLogsProducer,
                                                  std::shared_ptr<IFileLogsProducer> fileLogsProducer, logs_output::SINK logsOutputSink,
                                                  std::unique_ptr<IAsyncLogQueue> logMessageQueue)
    : mLogMessageQueue_(std::move(logMessageQueue)),
      mWorkerThread_{},
      mIsWorkerRunning_(false),
      mOutputMutex_{},
      mTimestampProducer_(timestamp_procducer),
      mConsoleLogsProducer_(std::move(consoleLogsProducer)),
      mFileLogsProducer_(fileLogsProducer),
      mLogsOutputSink_(logsOutputSink) {}

equinox::AsyncLogQueueEngine::~AsyncLogQueueEngine() {
    stopWorker();
}

void equinox::AsyncLogQueueEngine::processLogMessage(const std::string& messageToProcess) {
    mLogMessageQueue_->enqueue(messageToProcess);
}

void equinox::AsyncLogQueueEngine::startWorkerIfNeeded() {
    bool expected = false;
    if (!mIsWorkerRunning_.compare_exchange_strong(expected, true)) {
        return;
    }

    mWorkerThread_ = std::thread([this]() {
        std::vector<std::string> batch;
        while (true) {
            batch.clear();
            if (!mLogMessageQueue_->dequeue(batch, kDefaultBatchSize, kDefaultDequeueTimeoutMs)) {
                if (!mIsWorkerRunning_.load()) {
                    break;
                }
                continue;
            }

            for (const auto& message : batch) {
                {
                    std::lock_guard<std::mutex> lock(mOutputMutex_);
                    switch (mLogsOutputSink_) {
                        case logs_output::SINK::console:
                            mConsoleLogsProducer_->logMessage(message);
                            break;

                        case logs_output::SINK::file:
                            mFileLogsProducer_->logMessage(message);
                            break;

                        case logs_output::SINK::console_and_file:
                            mConsoleLogsProducer_->logMessage(message);
                            mFileLogsProducer_->logMessage(message);
                            break;
                    }
                }
            }
        }
    });
}

void equinox::AsyncLogQueueEngine::stopWorker() {
    if (!mIsWorkerRunning_.exchange(false)) {
        return;
    }

    mLogMessageQueue_->stop();
    if (mWorkerThread_.joinable()) {
        mWorkerThread_.join();
    }
}

void equinox::AsyncLogQueueEngine::setLogsOutputSink(logs_output::SINK logsOutputSink) {
    std::lock_guard<std::mutex> lock(mOutputMutex_);
    mLogsOutputSink_ = logsOutputSink;
}

void equinox::AsyncLogQueueEngine::flush() {
    std::lock_guard<std::mutex> lock(mOutputMutex_);
    mConsoleLogsProducer_->flush();
    mFileLogsProducer_->flush();
}