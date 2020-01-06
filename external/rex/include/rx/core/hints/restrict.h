#ifndef RX_CORE_HINTS_RESTRICT_H
#define RX_CORE_HINTS_RESTRICT_H
#include "rx/core/config.h"

#if defined(RX_COMPILER_GCC) || defined(RX_COMPILER_CLANG)
#define RX_HINT_RESTRICT __restrict__
#elif defined(RX_COMPILER_MSVC)
#define RX_HINT_RESTRICT __restrict
#else
#define RX_HINT_RESTRICT
#endif

#endif // RX_CORE_HINTS_RESTRICT_H
