# Equinox logging engine 2.2.84

**Logger with support logging to file, console or both. Six levels available:**
- Trace 
- Debug
- Info
- Warning
- Error
- Critical
- Off

## Table of contents

- [Features](#features)
- [Building for source](#building-for-source)
- [Code formatting](#code-formatting)
- [Install](#install)
- [Logging macros and `NDEBUG`](#logging-macros-and-ndebug)
- [Example](#example)
- [Configuration from a file](#configuration-from-a-file)
- [Colored logs preview](#colored-logs-preview-github-friendly)
- [Performance benchmark](#performance-benchmark)
- [Unit Test Coverage](#unit-test-coverage)

## Features

- settable log level
- settable output direction (console, file or both)
- configured build to shared/static lib

## Building for source

The project can be built with CMake and Make directly:

```sh
cmake CMakeLists.txt
make
```

For a clean, configurable build, use [`scripts/build.sh`](scripts/build.sh). The script removes the selected build directory before configuring it with CMake, builds with all available CPU cores, and stores the results under `build/`.

```sh
# Show available options
./scripts/build.sh --help

# Build the shared library in Debug mode (default build type)
./scripts/build.sh debug

# Build the Release library together with examples and tests
./scripts/build.sh release examples tests

# Build only the static library
./scripts/build.sh release --static

# Build both shared and static libraries
./scripts/build.sh debug --both
```

### Build script options

Build type and targets:

- `debug` or `release` selects the build type; `debug` is the default.
- `examples` builds the example applications.
- `tests` or `unit` builds and runs the unit tests.
- `format` formats the source files before building.

Library and test options:

- `--shared` builds the shared library (the default).
- `--static` builds only the static library.
- `--both` builds shared and static libraries in separate directories.
- `--skip-tests` builds the tests without running them.
- `--coverage` builds the Debug configuration, runs the tests, and generates an LCOV report.
- `--format-only` formats the source files and exits without building.
- `--clean` is accepted for compatibility; the selected build directory is always cleaned.

For example, test coverage can be generated with:

```sh
./scripts/build.sh debug tests --coverage
```

The main build artifacts are written to `build/Debug/` or `build/Release/`. When `--both` is used, they are written to `build/Debug-shared/` and `build/Debug-static/` (or the corresponding Release directories). The HTML coverage report is available at `build/Debug/coverage/html/index.html`.

Building tests requires GoogleTest. Coverage additionally requires `lcov` and `genhtml`; on Debian/Ubuntu, the script can try to install them using `apt-get`.

[Back to table of contents](#table-of-contents)

## Code formatting

Source files in `api/`, `include/`, `src/`, `examples/`, and unit tests in `tests/` (excluding `ThirdParty/` and build directories) are formatted according to [.clang-format](.clang-format) using `scripts/format.sh`:

```sh
# Format all source files and unit tests
./scripts/format.sh

# Check formatting compliance without modifying files
./scripts/format.sh --check

# Verbose output
./scripts/format.sh --verbose
```

Alternatively, code formatting can be triggered via `npm` or `build.sh`:

```sh
# Via npm
npm run format
npm run format:check

# Via build script
./scripts/build.sh --format-only
```

[Back to table of contents](#table-of-contents)

## Install
```sh
$ sudo make install (Ubuntu)
or
# make install
```

[Back to table of contents](#table-of-contents)

## Logging macros and `NDEBUG`

The project also provides convenience macros in `EquinoxLoggerMacros.h`:

```cpp
#include "EquinoxLoggerMacros.h"

SETUP_LOGGER(equinox::level::LOG_LEVEL::trace, std::string("equinox-macro_test"), equinox::logs_output::SINK::console_and_file, std::string("equinox.log"),
             3U * 1024U * 1024U, 5U);
LOG_TRACE("Example MACRO trace log no:    [%d]", 1);
LOG_DEBUG("Example MACRO debug log no:    [%d]", 2);
LOG_INFO("Example MACRO info log no:     [%d]", 3);
LOG_WARNING("Example MACRO warning log no:  [%d]", 4);
LOG_ERROR("Example MACRO error log no:    [%d]", 5);
LOG_CRITICAL("Example MACRO critical log no: [%d]", 6);

```

The macro behavior depends on the standard `NDEBUG` flag:

- If the build is a Debug configuration, `NDEBUG` is not defined, so `EQUINOX_TRACE(...)` and `EQUINOX_DEBUG(...)` call the normal logging API.
- If the build is a Release configuration, `NDEBUG` is typically defined by the compiler or by a build flag such as `-DNDEBUG` or the CMake Release preset, so those two macros compile to empty `do { } while (0)` statements.
- This removes the runtime cost of trace/debug formatting and evaluation in Release builds while keeping higher-priority logs available according to the configured logger level.

In other words, `trace` and `debug` are stripped out before formatting when `NDEBUG` is active, which gives a zero-cost path for these two levels in Release builds.

[Back to table of contents](#table-of-contents)

## Example:

Include "EquinoxLogger.hpp" to your source code:
```sh
See: examples/src/EquinoxLoggerExamples.cpp
```
```cpp
#include <iostream>

#include "EquinoxLogger.hpp"

int main(void) {
    equinox::setup(equinox::level::LOG_LEVEL::trace, std::string("equinox-test"), equinox::logs_output::SINK::console_and_file, std::string("equinox.log"),
                   3U * 1024U * 1024U, 5U);

    equinox::trace("Example trace log no:    [%d]", 1);
    equinox::debug("Example debug log no:    [%d]", 2);
    equinox::info("Example info log no:     [%d]", 3);
    equinox::warning("Example warning log no:  [%d]", 4);
    equinox::error("Example error log no:    [%d]", 5);
    equinox::critical("Example critical log no: [%d]", 6);

    SETUP_LOGGER(equinox::level::LOG_LEVEL::trace, std::string("equinox-macro_test"), equinox::logs_output::SINK::console_and_file, std::string("equinox.log"),
                 3U * 1024U * 1024U, 5U);
    LOG_TRACE("Example MACRO trace log no:    [%d]", 1);
    LOG_DEBUG("Example MACRO debug log no:    [%d]", 2);
    LOG_INFO("Example MACRO info log no:     [%d]", 3);
    LOG_WARNING("Example MACRO warning log no:  [%d]", 4);
    LOG_ERROR("Example MACRO error log no:    [%d]", 5);
    LOG_CRITICAL("Example MACRO critical log no: [%d]", 6);

    SETUP_FROM_CONFIG_FILE(std::string("config_file_example.txt"));
    LOG_TRACE("Example MACRO trace log no:    [%d]", 1);
    LOG_DEBUG("Example MACRO debug log no:    [%d]", 2);
    LOG_INFO("Example MACRO info log no:     [%d]", 3);
    LOG_WARNING("Example MACRO warning log no:  [%d]", 4);
    LOG_ERROR("Example MACRO error log no:    [%d]", 5);
    LOG_CRITICAL("Example MACRO critical log no: [%d]", 6);

    return 0;
}
```
```
Output:
 
[Thu Aug 27 15:14:31 2026][1787836471950][equinox-test][TRACE] Example trace log no:    [1]
[Thu Aug 27 15:14:31 2026][1787836471950][equinox-test][DEBUG] Example debug log no:    [2]
[Thu Aug 27 15:14:31 2026][1787836471950][equinox-test][INFO] Example info log no:     [3]
[Thu Aug 27 15:14:31 2026][1787836471950][equinox-test][WARNING] Example warning log no:  [4]
[Thu Aug 27 15:14:31 2026][1787836471951][equinox-test][ERROR] Example error log no:    [5]
[Thu Aug 27 15:14:31 2026][1787836471951][equinox-test][CRITICAL] Example critical log no: [6]

[Thu Aug 27 15:14:31 2026][1787836471951][equinox-macro_test][TRACE] Example MACRO trace log no:    [1]
[Thu Aug 27 15:14:31 2026][1787836471951][equinox-macro_test][DEBUG] Example MACRO debug log no:    [2]
[Thu Aug 27 15:14:31 2026][1787836471951][equinox-macro_test][INFO] Example MACRO info log no:     [3]
[Thu Aug 27 15:14:31 2026][1787836471951][equinox-macro_test][WARNING] Example MACRO warning log no:  [4]
[Thu Aug 27 15:14:31 2026][1787836471951][equinox-macro_test][ERROR] Example MACRO error log no:    [5]
[Thu Aug 27 15:14:31 2026][1787836471951][equinox-macro_test][CRITICAL] Example MACRO critical log no: [6]

[Thu Aug 27 15:14:31 2026][1787836471951][config-file-prefix][INFO] Example MACRO info log no:     [3]
[Thu Aug 27 15:14:31 2026][1787836471951][config-file-prefix][WARNING] Example MACRO warning log no:  [4]
[Thu Aug 27 15:14:31 2026][1787836471951][config-file-prefix][ERROR] Example MACRO error log no:    [5]
[Thu Aug 27 15:14:31 2026][1787836471951][config-file-prefix][CRITICAL] Example MACRO critical log no: [6]

```

[Back to table of contents](#table-of-contents)

## Configuration from a file

Instead of passing all setup parameters explicitly, the logger can be configured from a plain `key = value` text file using `equinox::setupFromConfigFile(configFilePath)` or the `SETUP_FROM_CONFIG_FILE(configFilePath)` macro.

```sh
See: examples/config_file_example.txt
```
```ini
# logger configuration file example

logLevel = 2
logPrefix = config-file-prefix
logsOutputSink = 0  #console
logFileName = /tmp/app.log
maxLogFileSizeBytes = 10485760
maxLogFiles = 5
```

```cpp
#include "EquinoxLogger.hpp"

SETUP_FROM_CONFIG_FILE(std::string("config_file_example.txt"));
// or: equinox::setupFromConfigFile("config_file_example.txt");
```

Supported keys:

| Key | Description | Values |
| --- | --- | --- |
| `logLevel` | minimal severity of logged messages | `0` trace, `1` debug, `2` info, `3` warning, `4` error, `5` critical, `6` off |
| `logPrefix` | prefix added to each log line | any string, without quotes |
| `logsOutputSink` | where logs are printed | `0` console, `1` file, `2` console and file |
| `logFileName` | path to the log file | any file path, without quotes |
| `maxLogFileSizeBytes` | max size of a single log file in bytes | positive integer |
| `maxLogFiles` | max number of rotated log files | positive integer |

Notes:
- Lines starting with `#` are treated as comments and ignored.
- Values are not quoted — `logPrefix = "name"` would keep the quotes as part of the prefix.
- If a key is missing, empty, or has an invalid value, the corresponding default value is used instead and the rest of the configuration is still applied.

[Back to table of contents](#table-of-contents)

GitHub README does not render terminal ANSI colors inside code blocks, so this preview uses colored badges for each level:

- ![TRACE](https://img.shields.io/badge/TRACE-6A5ACD) `[Mon Apr 3 15:43:39 2023][1680529419785][equinox-test][TRACE] Example trace log no: [1]`
- ![DEBUG](https://img.shields.io/badge/DEBUG-1E90FF) `[Mon Apr 3 15:43:39 2023][1680529419787][equinox-test][DEBUG] Example debug log no: [2]`
- ![INFO](https://img.shields.io/badge/INFO-2E8B57) `[Mon Apr 3 15:43:39 2023][1680529419787][equinox-test][INFO] Example info log no: [3]`
- ![WARNING](https://img.shields.io/badge/WARNING-DAA520) `[Mon Apr 3 15:43:39 2023][1680529419788][equinox-test][WARNING] Example warning log no: [4]`
- ![ERROR](https://img.shields.io/badge/ERROR-B22222) `[Mon Apr 3 15:43:39 2023][1680529419788][equinox-test][ERROR] Example error log no: [5]`
- ![CRITICAL](https://img.shields.io/badge/CRITICAL-8B0000) `[Mon Apr 3 15:43:39 2023][1680529419788][equinox-test][CRITICAL] Example critical log no: [6]`

[Back to table of contents](#table-of-contents)

## Performance benchmark

The repository runs a log-print throughput benchmark on every PR targeting `main`.

The benchmark is split by build type for comparison. The Release benchmark keeps a 10% regression guard, and the Debug benchmark is evaluated with the same baseline comparison policy without an extra percentage cushion.

This keeps the policy consistent with the measured behavior in the project, where the absolute timing of `Debug` and `Release` can vary and should not be treated as a fixed rule based only on optimization level.

What is measured:
- the benchmark executes a fixed number of log writes through the public logging API
- it measures the wall-clock time for the whole sequence using `std::chrono::steady_clock`
- it reports the result as milliseconds per run and stores it in the benchmark history file
- the generated chart compares the current result to the baseline and shows the 10% slowdown threshold

The exact benchmark test looks like this:
```cpp
TEST(PerformanceBenchmark, LogPrintThroughput) {
    std::filesystem::remove(kBenchmarkLogPath);

    ASSERT_TRUE(equinox::setup(equinox::level::LOG_LEVEL::info,
                               "PerformanceBenchmark",
                               equinox::logs_output::SINK::file,
                               kBenchmarkLogPath,
                               32U * 1024U * 1024U,
                               2U));

    const auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < kBenchmarkIterations; ++i) {
        equinox::info("benchmark message %d | value=%d | tag=%s", i, i * 11, "perf");
    }

    equinox::flush();

    const auto end = std::chrono::steady_clock::now();
    const auto elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "BENCHMARK_RESULT: {\"test\":\"LogPrintThroughput\",\"iterations\":"
              << kBenchmarkIterations
              << ",\"elapsed_ms\":" << elapsed_ms << "}" << std::endl;

    EXPECT_GT(elapsed_ms, 0.0);
}
```

This test prints the measured time as a JSON-like result, which is then consumed by the CI script to compare with the baseline and enforce the configured threshold for that build type.

Failure rule:
- the Release benchmark fails when the current runtime is slower than the baseline by more than 10%
- the Debug benchmark follows the same comparison rule without a separate threshold modifier
- the rule is enforced in CI and the check exits non-zero if the slowdown exceeds the threshold

The benchmark is implemented in the `PerformanceBenchmark.LogPrintThroughput` GoogleTest and the chart is generated by `scripts/performance_benchmark.py`.

### Release benchmark (main chart)
Measured on the Release build. Threshold: 10%.

The latest Release benchmark is published from GitHub Pages and refreshed on each successful main-branch CI run, without committing generated images to the repository.

Live benchmark dashboard: https://jwolak.github.io/Equinox/

> This chart is regenerated only after a successful merge into `main`. Pull request runs validate the benchmark and threshold checks, but they do not publish the main Pages site.

![Performance benchmark chart](https://jwolak.github.io/Equinox/performance-benchmark-Release.svg)

> This image reflects the latest successful build from the main branch. PR runs do not deploy the Pages site; they only validate the benchmark and test jobs.

### Debug benchmark (additional comparison)
Measured on the Debug build using the same baseline comparison rule as the Release benchmark.

![Debug performance benchmark](https://jwolak.github.io/Equinox/performance-benchmark-Debug.svg)

> The benchmark charts are published as temporary CI artifacts and also deployed to the GitHub Pages site for the latest successful build.
>
> - `performance-benchmark-Release.svg`
> - `performance-throughput-by-sink-Release.svg`
> - `performance-benchmark-Debug.svg`
> - `performance-throughput-by-sink-Debug.svg`

To generate the charts locally:
```sh
./scripts/run_performance_benchmark.sh \
  --binary build/Debug/tests/EquinoxLoggerTests.x86 \
  --output-dir docs/images \
  --chart-name performance-benchmark-Release.svg \
  --sink-chart-name performance-throughput-by-sink-Release.svg \
  --history-file benchmarks/performance-history-Release.json \
  --threshold 10 \
  --build-label "Release"

./scripts/run_performance_benchmark.sh \
  --binary build/Debug/tests/EquinoxLoggerTests.x86 \
  --output-dir docs/images \
  --chart-name performance-benchmark-Debug.svg \
  --sink-chart-name performance-throughput-by-sink-Debug.svg \
  --history-file benchmarks/performance-history-Debug.json \
  --threshold 10 \
  --build-label "Debug"
```

The output is saved separately for each build type, with Release used as the primary benchmark shown in the main README and Debug kept as an additional comparison chart.

### Throughput by message length and sink

The project also includes a second benchmark that measures how many log entries per second can be processed for different message sizes and output sinks.

For each combination of message length and sink (`console`, `file`, `console_and_file`) the test records the throughput in logs/sec and renders a grouped chart.

In CI, the noisy `console` and `console_and_file` variants are intentionally skipped to keep the job logs readable and to avoid flooding GitHub Actions output. The published CI chart therefore focuses on the `file` sink for the automated regression run, while the full local benchmark still includes all sink variants.

Both benchmark charts are published as CI artifacts for every PR, so the throughput-by-sink result is kept alongside the main regression chart.

![Throughput by message length and sink](docs/images/performance-throughput-by-sink.svg)

To generate this chart locally:
```sh
./scripts/run_performance_benchmark.sh \
  --binary build/Debug/tests/EquinoxLoggerTests.x86 \
  --output-dir docs/images \
  --chart-name performance-throughput-by-sink.svg \
  --threshold 10
```

This benchmark is useful for understanding how output destination and log message size affect throughput in real workloads.

### CI benchmark policy

The benchmark suite is intentionally a bit different in CI than on a developer machine.

- full sink-by-length throughput benchmarks are kept for local validation
- in GitHub Actions, the noisy `console` and `console_and_file` variants are skipped to avoid flooding the job logs
- CI still runs a compact `file`-sink throughput check to preserve meaningful regression coverage without causing log spam

This keeps the PR checks readable while still measuring the most important performance path under automation.

[Back to table of contents](#table-of-contents)

## Unit Test Coverage

Coverage measured with [LCOV](https://github.com/linux-test-project/lcov) on 2026-05-01:

| Metric             | Rate   | Total | Hit |
|--------------------|--------|-------|-----|
| Lines              | 96.2 % | 417   | 401 |
| Functions          | 92.3 % | 104   | 96  |

![Unit Test Coverage Report](docs/images/equinox-logger-ut-coverage.PNG)

To regenerate the report:

[Back to table of contents](#table-of-contents)
```sh
./scripts/coverage.sh -o
```

## Log rotation

- When the log file reaches the configured max size, the current log is renamed to a rotated file and a new file is created.
- Rotated files use the scheme logs_1.log, logs_2.log, ... up to the configured max number of files, then wrap around.
- Rotation is enabled when both max size and max files are greater than 0.

## License
**BSD 3-Clause License**
<br/>Copyright (c) 2023 - 2026, Janusz Wolak
<br/>All rights reserved.
