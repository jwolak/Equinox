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

#include <gtest/gtest.h>

#include <future>
#include <iostream>
#include <vector>

#include "EquinoxLogger.h"

void LogTraceInThreadOne() {
    equinox::trace("%s", "Message in Trace from thread one");
}

void LogDebugInThreadTwo() {
    equinox::debug("%s", "Message in Debug from thread two");
}

TEST(MultipleThreadsTest, Log_Trace_And_Debug_Then_Trace_Printed_As_First) {
    equinox::setup(equinox::level::LOG_LEVEL::trace, std::string("MultipleThreadsTest"), equinox::logs_output::SINK::console);

    auto fut1 = std::async(std::launch::async, LogTraceInThreadOne);
    auto fut2 = std::async(std::launch::async, LogDebugInThreadTwo);

    fut1.wait();
    fut2.wait();
}

TEST(MultipleThreadsTest, Repeated_Concurrent_Logging_Should_Not_Crash) {
    for (int iteration = 0; iteration < 25; ++iteration) {
        equinox::setup(equinox::level::LOG_LEVEL::trace, std::string("MultipleThreadsStressTest"), equinox::logs_output::SINK::console);

        std::vector<std::future<void>> tasks;
        tasks.reserve(8);
        for (int i = 0; i < 8; ++i) {
            tasks.emplace_back(std::async(std::launch::async, [i]() {
                if (i % 2 == 0) {
                    equinox::trace("%s", "Concurrent trace message");
                } else {
                    equinox::debug("%s", "Concurrent debug message");
                }
            }));
        }

        for (auto& task : tasks) {
            task.wait();
        }

        equinox::flush();
    }

    SUCCEED();
}
