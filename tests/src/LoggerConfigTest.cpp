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

#include <gtest/gtest.h>

#include "LoggerConfig.h"

namespace logger_config_test {
    using namespace equinox;

    class LoggerConfigTest : public ::testing::Test {
       public:
        LoggerConfigTest() : logger_config{} {}

        LoggerConfig logger_config;
    };

    TEST_F(LoggerConfigTest, Default_Constructor_Initializes_Logger_Config_And_Defaults_Are_Set) {}

    TEST_F(LoggerConfigTest, Compare_Same_Logger_Config_Struct_And_True_Returned) {}

    TEST_F(LoggerConfigTest, Compare_Different_Logger_Config_Struct_And_False_Returned) {}

    TEST_F(LoggerConfigTest, Set_Logger_Level_From_Int_Zero_And_Trace_Logger_Level_Is_Set) {}

    TEST_F(LoggerConfigTest, Set_Logger_Level_From_Int_One_And_Debug_Logger_Level_Is_Set) {}

    TEST_F(LoggerConfigTest, Set_Logger_Level_From_Int_Two_And_Info_Logger_Level_Is_Set) {}

    TEST_F(LoggerConfigTest, Set_Logger_Level_From_Int_Three_And_Warn_Logger_Level_Is_Set) {}

    TEST_F(LoggerConfigTest, Set_Logger_Level_From_Int_Four_And_Error_Logger_Level_Is_Set) {}

    TEST_F(LoggerConfigTest, Set_Logger_Level_From_Int_Five_And_Critical_Logger_Level_Is_Set) {}

    TEST_F(LoggerConfigTest, Set_Logger_Level_From_Int_Six_And_Off_Logger_Level_Is_Set) {}

    TEST_F(LoggerConfigTest, Set_Logger_Level_From_Int_Invalid_And_Default_Logger_Level_Is_Set) {}

    TEST_F(LoggerConfigTest, Set_Sink_Output_From_Int_Zero_And_Console_Sink_Is_Set) {}

    TEST_F(LoggerConfigTest, Set_Sink_Output_From_Int_One_And_File_Sink_Is_Set) {}

    TEST_F(LoggerConfigTest, Set_Sink_Output_From_Int_Invalid_And_Default_Sink_Is_Set) {}
}  // namespace logger_config_test