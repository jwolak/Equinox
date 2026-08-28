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

#pragma once

#include <string>
#include <string_view>

#include "EquinoxLoggerCommon.h"
#include "IColorFormatter.h"

namespace equinox {

    class ColorFormatter : public IColorFormatter {
       public:
        /**
         * Extracts the log level from a formatted message (looks for [TRACE], [DEBUG], etc.)
         *
         * @param message The formatted message containing the level prefix
         * @return The extracted LOG_LEVEL or info if not found
         */
        level::LOG_LEVEL extractLevelFromMessage(const std::string& message) override;

        /**
         * Applies ANSI color codes to a message based on its log level prefix
         *
         * @param message The message containing [LEVEL] prefix
         * @param color The ANSI color code
         * @return The message wrapped with appropriate ANSI color codes
         */
        std::string applyConsoleColors(const std::string& message, std::string_view color) override;

        /**
         * Gets the ANSI color code for the given log level
         *
         * @param logLevel The log level
         * @return The ANSI color code string
         */
        std::string_view getColorForLevel(level::LOG_LEVEL logLevel) override;
    };

} /*namespace equinox*/
