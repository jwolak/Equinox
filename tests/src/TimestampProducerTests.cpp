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

#include <iostream>
#include <memory>

#include "TimestampProducer.h"

namespace time_stampproducer_tests {

    class TimestampProducerTests : public ::testing::Test {
       public:
        TimestampProducerTests() : mTimestampProducer{std::make_unique<equinox::TimestampProducer>()} {}

        std::unique_ptr<equinox::ITimestampProducer> mTimestampProducer;
    };

    TEST_F(TimestampProducerTests, Call_getTimestamp_And_No_Failure) {
        ASSERT_NO_FATAL_FAILURE(mTimestampProducer->getTimestamp());
    }

    TEST_F(TimestampProducerTests, Call_getTimestampInUs_And_No_Failure) {
        ASSERT_NO_FATAL_FAILURE(mTimestampProducer->getTimestampInUs());
    }

    TEST_F(TimestampProducerTests, Call_getTimestamp_And_Print_Result) {
        std::cout << mTimestampProducer->getTimestamp() << std::endl;
    }

    TEST_F(TimestampProducerTests, Call_getTimestampInUs_And_Print_Result) {
        std::cout << mTimestampProducer->getTimestampInUs() << std::endl;
    }

} /*namespace time_stampproducer_tests*/
