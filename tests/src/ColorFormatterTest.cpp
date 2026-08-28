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

#include <thread>

#include "ColorFormatter.h"

namespace async_log_queue_test {

    using namespace equinox;
    using namespace ::testing;

    namespace {
        static constexpr std::string_view kColorReset = "\033[0m";
        static constexpr std::string_view kColorCyan = "\033[36m";
        static constexpr std::string_view kColorGreen = "\033[32m";
        static constexpr std::string_view kColorYellow = "\033[33m";
        static constexpr std::string_view kColorRed = "\033[31m";
        static constexpr std::string_view kColorMagenta = "\033[35m";
        static constexpr std::string_view kColorDefault = "";
    }  // namespace

    class ColorFormatterTest : public ::testing::Test {
       public:
        ColorFormatterTest() : color_formatter{} {}

        ColorFormatter color_formatter;
    };

    TEST_F(ColorFormatterTest, Try_Get_Color_For_Invalid_Level_And_Default_Color_Returned) {
        ASSERT_EQ(color_formatter.getColorForLevel(static_cast<level::LOG_LEVEL>(999)), "");
    }

    TEST_F(ColorFormatterTest, Get_Color_For_Trace_Level_And_Trace_Color_Returned) {
        ASSERT_EQ(color_formatter.getColorForLevel(level::LOG_LEVEL::trace), kColorCyan);
    }

    TEST_F(ColorFormatterTest, Get_Color_For_Debug_Level_And_Debug_Color_Returned) {
        ASSERT_EQ(color_formatter.getColorForLevel(level::LOG_LEVEL::debug), kColorGreen);
    }

    TEST_F(ColorFormatterTest, Get_Color_For_Info_Level_And_Info_Color_Returned) {
        ASSERT_EQ(color_formatter.getColorForLevel(level::LOG_LEVEL::info), kColorDefault);
    }

    TEST_F(ColorFormatterTest, Get_Color_For_Warning_Level_And_Warning_Color_Returned) {
        ASSERT_EQ(color_formatter.getColorForLevel(level::LOG_LEVEL::warning), kColorYellow);
    }

    TEST_F(ColorFormatterTest, Get_Color_For_Error_Level_And_Error_Color_Returned) {
        ASSERT_EQ(color_formatter.getColorForLevel(level::LOG_LEVEL::error), kColorRed);
    }

    TEST_F(ColorFormatterTest, Get_Color_For_Critical_Level_And_Critical_Color_Returned) {
        ASSERT_EQ(color_formatter.getColorForLevel(level::LOG_LEVEL::critical), kColorMagenta);
    }

    TEST_F(ColorFormatterTest, Try_Get_Color_For_Off_Level_And_Default_Color_Returned) {
        ASSERT_EQ(color_formatter.getColorForLevel(level::LOG_LEVEL::off), kColorDefault);
    }

    TEST_F(ColorFormatterTest, Extract_Level_From_Trace_Message_And_Trace_Level_Returned) {
        ASSERT_EQ(color_formatter.extractLevelFromMessage("[TRACE] This is a trace message"), level::LOG_LEVEL::trace);
    }

    TEST_F(ColorFormatterTest, Extract_Level_From_Debug_Message_And_Debug_Level_Returned) {
        ASSERT_EQ(color_formatter.extractLevelFromMessage("[DEBUG] This is a debug message"), level::LOG_LEVEL::debug);
    }

    TEST_F(ColorFormatterTest, Extract_Level_From_Info_Message_And_Info_Level_Returned) {
        ASSERT_EQ(color_formatter.extractLevelFromMessage("[INFO] This is an info message"), level::LOG_LEVEL::info);
    }

    TEST_F(ColorFormatterTest, Extract_Level_From_Warning_Message_And_Warning_Level_Returned) {
        ASSERT_EQ(color_formatter.extractLevelFromMessage("[WARNING] This is a warning message"), level::LOG_LEVEL::warning);
    }

    TEST_F(ColorFormatterTest, Extract_Level_From_Error_Message_And_Error_Level_Returned) {
        ASSERT_EQ(color_formatter.extractLevelFromMessage("[ERROR] This is an error message"), level::LOG_LEVEL::error);
    }

    TEST_F(ColorFormatterTest, Extract_Level_From_Critical_Message_And_Critical_Level_Returned) {
        ASSERT_EQ(color_formatter.extractLevelFromMessage("[CRITICAL] This is a critical message"), level::LOG_LEVEL::critical);
    }

    TEST_F(ColorFormatterTest, Extract_Level_From_Message_Without_Level_And_Info_Level_Returned) {
        ASSERT_EQ(color_formatter.extractLevelFromMessage("This message has no level prefix"), level::LOG_LEVEL::info);
    }

    TEST_F(ColorFormatterTest, Apply_Console_Colors_To_Message_And_Message_Wrapped_With_Color_Codes_Returned) {
        std::string message = "This is a test message";
        std::string expected = std::string(kColorRed) + message + std::string(kColorReset);

        ASSERT_EQ(color_formatter.applyConsoleColors(message, kColorRed), expected);
    }
}  // namespace async_log_queue_test