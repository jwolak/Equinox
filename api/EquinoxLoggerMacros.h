#include "EquinoxLogger.h"

#if defined(NDEBUG)
#define LOG_TRACE(...) \
    do {               \
    } while (0)
#define LOG_DEBUG(...) \
    do {               \
    } while (0)
#else
#define LOG_TRACE(...) equinox::trace(__VA_ARGS__)
#define LOG_DEBUG(...) equinox::debug(__VA_ARGS__)
#endif

#define LOG_ERROR(...)    equinox::error(__VA_ARGS__)
#define LOG_WARNING(...)  equinox::warning(__VA_ARGS__)
#define LOG_INFO(...)     equinox::info(__VA_ARGS__)
#define LOG_CRITICAL(...) equinox::critical(__VA_ARGS__)

#define SETUP_LOGGER(logLevel, logPrefix, logsOutputSink, logFileName, maxLogFileSizeBytes, maxLogFiles) \
    equinox::setup(logLevel, logPrefix, logsOutputSink, logFileName, maxLogFileSizeBytes, maxLogFiles)
