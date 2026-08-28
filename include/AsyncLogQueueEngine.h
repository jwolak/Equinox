#pragma once

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

#include <atomic>
#include <mutex>
#include <string>
#include <thread>

#include "AsyncLogQueue.h"
#include "ConsoleLogsProducer.h"
#include "EquinoxLoggerCommon.h"
#include "FileLogsProducer.h"
#include "IAsyncLogQueueEngine.h"
#include "TimestampProducer.h"

namespace equinox {

    class AsyncLogQueueEngine : public IAsyncLogQueueEngine {
       public:
        explicit AsyncLogQueueEngine(std::shared_ptr<ITimestampProducer> timestamp_procducer, std::shared_ptr<IFileLogsProducer> fileLogsProducer,
                                     logs_output::SINK logsOutputSink);
        ~AsyncLogQueueEngine();
        void processLogMessage(const std::string& messageToProcess);
        void stopWorker();
        void startWorkerIfNeeded();
        void setLogsOutputSink(logs_output::SINK logsOutputSink);
        void flush();

       protected:
        /* For tests purpose */
        AsyncLogQueueEngine(std::shared_ptr<ITimestampProducer> timestamp_procducer, std::unique_ptr<IConsoleLogsProducer> consoleLogsProducer,
                            std::shared_ptr<IFileLogsProducer> fileLogsProducer, logs_output::SINK logsOutputSink,
                            std::unique_ptr<IAsyncLogQueue> logMessageQueue);

       private:
        std::unique_ptr<IAsyncLogQueue> mLogMessageQueue_;
        std::thread mWorkerThread_;
        std::atomic<bool> mIsWorkerRunning_;
        std::mutex mOutputMutex_;

        std::shared_ptr<ITimestampProducer> mTimestampProducer_;
        std::unique_ptr<IConsoleLogsProducer> mConsoleLogsProducer_;
        std::shared_ptr<IFileLogsProducer> mFileLogsProducer_;
        logs_output::SINK mLogsOutputSink_;
    };
}  // namespace equinox
