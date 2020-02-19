#ifndef RX_CORE_HINTS_UNLIKELY_H
#define RX_CORE_HINTS_UNLIKELY_H
#include "rx/core/config.h" // RX_COMPILER_{GCC,CLANG}

// # Unlikely
//
// Hint to indicate that the given conditional is unlikely to evaluate true,
// enabling better code generation of branches.
#if defined(RX_COMPILER_GCC) || defined(RX_COMPILER_CLANG)
#define RX_HINT_UNLIKELY(_x) __builtin_expect(!!(_x), 0)
#else
#define RX_HINT_UNLIKELY(_x) (_x)
#endif // defined(RX_COMPILER_GCC) || defined(RX_COMPILER_CLANG)

#endif // RX_CORE_HINTS_UNLIKELY_H
