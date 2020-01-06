#ifndef RX_CORE_HINTS_UNREACHABLE_H
#define RX_CORE_HINTS_UNREACHABLE_H
#include "rx/core/config.h"

#if defined(RX_COMPILER_GCC) || defined(RX_COMPILER_CLANG)
#define RX_HINT_UNREACHABLE() \
  __builtin_unreachable()
#elif defined(RX_COMPILER_MSVC)
#define RX_HINT_UNREACHABLE() __assume(0)
#else
#define RX_HINT_UNREACHABLE()
#endif // defined(RX_COMPILER_GCC) || defined(RX_COMPILER_CLANG)

#endif // RX_CORE_HINTS_UNREACHABLE_H
