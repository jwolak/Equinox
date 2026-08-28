/*
 * EquinoxLoggerCommon.h
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

#ifndef API_EQUINOXLOGGERCOMMON_H_
#define API_EQUINOXLOGGERCOMMON_H_

#include <cstddef>
#include <string>

#if defined(EQUINOX_SHARED_SHARED_LIB)
#undef EQUINOX_HEADER_ONLY
#define EQUINOX_API __attribute__((visibility("default")))
#define EQUINOX_INLINE
#else  // !defined(EQUINOX_SHARED_SHARED_LIB)
#define EQUINOX_API
#define EQUINOX_HEADER_ONLY
#define EQUINOX_INLINE inline
#endif  // #ifdef EQUINOX_SHARED_SHARED_LIB

#define EQUINOX_LEVEL_TRACE 0
#define EQUINOX_LEVEL_DEBUG 1
#define EQUINOX_LEVEL_INFO 2
#define EQUINOX_LEVEL_WARNING 3
#define EQUINOX_LEVEL_ERROR 4
#define EQUINOX_LEVEL_CRITICAL 5
#define EQUINOX_LEVEL_OFF 6

#define EQUINOX_SINK_CONSOLE 0
#define EQUINOX_SINK_FILE 1
#define EQUINOX_SINK_CONSOLE_AND_FILE 2

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

namespace equinox {

const std::string kLogFileName = "logs.log";
const std::string kLogDefaultPrefix = "log_prefix_not_set";
const std::size_t kDefaultMaxLogFileSizeBytes = 3U * 1024U * 1024U;
const std::size_t kDefaultMaxLogFiles = 5U;

namespace level {
enum class LOG_LEVEL : int {
  trace = EQUINOX_LEVEL_TRACE,
  debug = EQUINOX_LEVEL_DEBUG,
  info = EQUINOX_LEVEL_INFO,
  warning = EQUINOX_LEVEL_WARNING,
  error = EQUINOX_LEVEL_ERROR,
  critical = EQUINOX_LEVEL_CRITICAL,
  off = EQUINOX_LEVEL_OFF
};
} /*namespace level*/

namespace logs_output {
enum class SINK : int { console = EQUINOX_SINK_CONSOLE, file = EQUINOX_SINK_FILE, console_and_file = EQUINOX_SINK_CONSOLE_AND_FILE };
} /*namespace logs_output*/

constexpr level::LOG_LEVEL kDefaultLogLevel = level::LOG_LEVEL::info;
constexpr logs_output::SINK kDefaultLogsOutputSink = logs_output::SINK::console;

} /*namespace equinox*/

#endif /* API_EQUINOXLOGGERCOMMON_H_ */
