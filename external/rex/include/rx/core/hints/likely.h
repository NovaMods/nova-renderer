#ifndef RX_CORE_HINTS_LIKELY_H
#define RX_CORE_HINTS_LIKELY_H
#include "rx/core/config.h" // RX_COMPILER_{GCC,CLANG}

// # Likely
//
// Hint to indicate that the given conditional is likely to evaluate true,
// enabling better code generation of branches.
#if defined(RX_COMPILER_GCC) || defined(RX_COMPILER_CLANG)
#define RX_HINT_LIKELY(_x) __builtin_expect(!!(_x), 1)
#else
#define RX_HINT_LIKELY(_x) (_x)
#endif // defined(RX_COMPILER_GCC) || defined(RX_COMPILER_CLANG)

#endif // RX_CORE_HINTS_LIKELY_H
