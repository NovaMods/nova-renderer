#ifndef RX_CORE_HINTS_NO_INLINE_H
#define RX_CORE_HINTS_NO_INLINE_H
#include "rx/core/config.h" // RX_COMPILER_{GCC,CLANG,MSVC}

// # Prevent inlining
//
// Hints to the compiler that a given function should not be inlined.
#if defined(RX_COMPILER_GCC) || defined(RX_COMPILER_CLANG)
#define RX_HINT_NO_INLINE __attribute__((noinline))
#elif defined(RX_COMPILER_MSVC)
#define RX_HINT_NO_INLINE
#else
#define RX_HINT_NO_INLINE
#endif

#endif // RX_CORE_HINTS_NO_INLINE_H
