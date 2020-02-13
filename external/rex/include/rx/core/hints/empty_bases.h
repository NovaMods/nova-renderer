#ifndef RX_CORE_HINTS_EMPTY_BASES_H
#define RX_CORE_HINTS_EMPTY_BASES_H

#include "rx/core/config.h"

#if defined(RX_PLATFORM_WINDOWS)
#define RX_HINT_EMPTY_BASES __declspec(empty_bases)
#else
#define RX_HINT_EMPTY_BASES
#endif

#endif // RX_CORE_HINTS_EMPTY_BASES_H
