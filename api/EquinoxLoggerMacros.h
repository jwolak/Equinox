#include "EquinoxLogger.h"

#if defined(NDEBUG)
#define EQUINOX_TRACE(...) \
    do {                   \
    } while (0)
#define EQUINOX_DEBUG(...) \
    do {                   \
    } while (0)
#else
#define EQUINOX_TRACE(...) ::equinox::trace(__VA_ARGS__)
#define EQUINOX_DEBUG(...) ::equinox::debug(__VA_ARGS__)
#endif

#define EQUINOX_ERROR(...)    ::equinox::error(__VA_ARGS__)
#define EQUINOX_WARNING(...)  ::equinox::warning(__VA_ARGS__)
#define EQUINOX_INFO(...)     ::equinox::info(__VA_ARGS__)
#define EQUINOX_CRITICAL(...) ::equinox::critical(__VA_ARGS__)
