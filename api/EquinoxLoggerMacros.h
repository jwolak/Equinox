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