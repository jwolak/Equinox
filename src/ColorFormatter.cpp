/*
 * ColorFormatter.cpp
 *
 *  Created on: 2026
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

#include "ColorFormatter.h"

namespace equinox {

    namespace {
        static constexpr std::string_view kColorReset = "\033[0m";
        static constexpr std::string_view kColorCyan = "\033[36m";
        static constexpr std::string_view kColorGreen = "\033[32m";
        static constexpr std::string_view kColorYellow = "\033[33m";
        static constexpr std::string_view kColorRed = "\033[31m";
        static constexpr std::string_view kColorMagenta = "\033[35m";
        static constexpr std::string_view kColorDefault = "";
    }  // namespace

    std::string_view ColorFormatter::getColorForLevel(level::LOG_LEVEL logLevel) {
        switch (logLevel) {
            case level::LOG_LEVEL::trace:
                return kColorCyan;
            case level::LOG_LEVEL::debug:
                return kColorGreen;
            case level::LOG_LEVEL::info:
                return kColorDefault;
            case level::LOG_LEVEL::warning:
                return kColorYellow;
            case level::LOG_LEVEL::error:
                return kColorRed;
            case level::LOG_LEVEL::critical:
                return kColorMagenta;
            case level::LOG_LEVEL::off:
            default:
                return kColorDefault;
        }
    }

    level::LOG_LEVEL ColorFormatter::extractLevelFromMessage(const std::string& message) {
        if (message.find("[TRACE]") != std::string::npos)
            return level::LOG_LEVEL::trace;
        if (message.find("[DEBUG]") != std::string::npos)
            return level::LOG_LEVEL::debug;
        if (message.find("[INFO]") != std::string::npos)
            return level::LOG_LEVEL::info;
        if (message.find("[WARNING]") != std::string::npos)
            return level::LOG_LEVEL::warning;
        if (message.find("[ERROR]") != std::string::npos)
            return level::LOG_LEVEL::error;
        if (message.find("[CRITICAL]") != std::string::npos)
            return level::LOG_LEVEL::critical;

        // Default to info if not found
        return level::LOG_LEVEL::info;
    }

    std::string ColorFormatter::applyConsoleColors(const std::string& message, std::string_view color) {
        // If color is empty (default), return message as-is
        if (color.empty()) {
            return message;
        }

        return std::string(color) + message + std::string(kColorReset);
    }

} /*namespace equinox*/
