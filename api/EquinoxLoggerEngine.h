/*
 * EquinoxLoggerEngine.h
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

#ifndef API_EQUINOXLOGGERENGINE_H_
#define API_EQUINOXLOGGERENGINE_H_

#include <fmt/printf.h>

#include <iostream>
#include <memory>
#include <mutex>
#include <string>

#include "EquinoxLoggerCommon.h"
#include "IEquinoxLoggerEngineImpl.h"

namespace equinox {

    class EquinoxLoggerEngineImpl;

    class EQUINOX_API EquinoxLoggerEngine {
       public:
        static EquinoxLoggerEngine& getInstance();

        EquinoxLoggerEngine(EquinoxLoggerEngine&) = delete;
        EquinoxLoggerEngine(EquinoxLoggerEngine&&) = delete;
        void operator=(const EquinoxLoggerEngine&) = delete;
        void operator=(const EquinoxLoggerEngine&&) = delete;

        template <typename... Args>
        void log(level::LOG_LEVEL msgLevel, const std::string& msgFormat, Args&&... args) {
            try {
                std::string formattedMessage = fmt::sprintf(msgFormat, std::forward<Args>(args)...);
                if (formattedMessage.size() > 4095U) {
                    formattedMessage.resize(4095U);
                }
                std::lock_guard<std::mutex> lock(mEngineMutex_);
                mEquinoxLoggerEngineImpl_->logMessage(msgLevel, formattedMessage);
            } catch (const fmt::format_error& ex) {
                std::cout << "[EquinoxLoggerEngine] Message formatting error: " << ex.what() << std::endl;
            } catch (const std::exception& ex) {
                std::cout << "[EquinoxLoggerEngine] Message formatting error: " << ex.what() << std::endl;
            }
        }

        bool setup(equinox::level::LOG_LEVEL logLevel, const std::string& logPrefix, equinox::logs_output::SINK logsOutputSink,
                   const std::string& logFileName = kLogFileName, std::size_t maxLogFileSizeBytes = kDefaultMaxLogFileSizeBytes,
                   std::size_t maxLogFiles = kDefaultMaxLogFiles);
        void changeLevel(level::LOG_LEVEL logLevel);
        bool changeLogsOutputSink(logs_output::SINK logsOutputSink);
        void flush();

       protected:
        EquinoxLoggerEngine();
        EquinoxLoggerEngine(std::unique_ptr<IEquinoxLoggerEngineImpl> mEquinoxLoggerEngineImpl);

       private:
        std::unique_ptr<IEquinoxLoggerEngineImpl> mEquinoxLoggerEngineImpl_;
        mutable std::mutex mEngineMutex_;
    };

} /*namespace equinox*/

#endif /* API_EQUINOXLOGGERENGINE_H_ */
