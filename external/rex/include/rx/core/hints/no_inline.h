#ifndef RX_CORE_HINTS_NO_INLINE_H
#define RX_CORE_HINTS_NO_INLINE_H
#include "rx/core/config.h"

#if defined(RX_COMPILER_GCC) || defined(RX_COMPILER_CLANG)
#define RX_HINT_NO_INLINE __attribute__((noinline))
#elif defined(RX_COMPILER_MSVC)
#define RX_HINT_NO_INLINE
#else
#define RX_HINT_NO_INLINE
#endif

#endif // RX_CORE_HINTS_NO_INLINE_H
