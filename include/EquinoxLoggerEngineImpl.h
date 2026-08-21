/*
 * EquinoxLoggerEngineImpl.h
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

#ifndef INCLUDE_EQUINOXLOGGERENGINEIMPL_H_
#define INCLUDE_EQUINOXLOGGERENGINEIMPL_H_

#include <atomic>
#include <memory>
#include <string>
#include <thread>

#include "AsyncLogQueue.h"
#include "AsyncLogQueueEngine.h"
#include "ColorFormatter.h"
#include "ConsoleLogsProducer.h"
#include "EquinoxLoggerCommon.h"
#include "FileLogsProducer.h"
#include "IEquinoxLoggerEngineImpl.h"
#include "TimestampProducer.h"

namespace equinox {

    class EQUINOX_API EquinoxLoggerEngineImpl : public IEquinoxLoggerEngineImpl {
       public:
        EquinoxLoggerEngineImpl();
        void logMessage(level::LOG_LEVEL msgLevel, const std::string& formatedOutputMessage) override;
        bool shouldLog(level::LOG_LEVEL msgLevel) const override;
        bool setup(level::LOG_LEVEL logLevel, const std::string& logPrefix, equinox::logs_output::SINK logsOutputSink, const std::string& logFileName,
                   std::size_t maxLogFileSizeBytes, std::size_t maxLogFiles) override;
        bool setupFromConfigFile(const std::string& configFilePath) override;
        void changeLevel(level::LOG_LEVEL logLevel) override;
        bool changeLogsOutputSink(logs_output::SINK logsOutputSink) override;
        void flush() override;

       protected:
        EquinoxLoggerEngineImpl(std::shared_ptr<ITimestampProducer> mTimestampProducer, std::shared_ptr<IFileLogsProducer> mFileLogsProducer,
                                std::unique_ptr<IAsyncLogQueueEngine> mAsyncLogQueueEngine);
        const std::string& getLogPrefix() const;
        level::LOG_LEVEL getLogLevel() const;
        const std::string& getLogFileName() const;
        std::size_t getMaxLogFileSizeBytes() const;
        std::size_t getMaxLogFiles() const;

       private:
        std::string mLogPrefix_;
        level::LOG_LEVEL mLogLevel_;
        std::string mLogFileName_;
        std::size_t mMaxLogFileSizeBytes_;
        std::size_t mMaxLogFiles_;
        std::shared_ptr<ITimestampProducer> mTimestampProducer_;
        std::shared_ptr<IFileLogsProducer> mFileLogsProducer_;
        std::unique_ptr<IAsyncLogQueueEngine> mAsyncLogQueueEngine_;
    };

} /*namespace equinox*/

#endif /* INCLUDE_EQUINOXLOGGERENGINEIMPL_H_ */
