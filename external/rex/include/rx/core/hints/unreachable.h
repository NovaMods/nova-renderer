#ifndef RX_CORE_HINTS_UNREACHABLE_H
#define RX_CORE_HINTS_UNREACHABLE_H
#include "rx/core/config.h" // RX_COMPILER_{GCC,CLANG,MSVC}

// # Unreachable
//
// Hint to indicate that some scope is tautologically unreachable. This hint
// is usually used to suppress warnings in contexts that the compiler cannot
// deduce.
#if defined(RX_COMPILER_GCC) || defined(RX_COMPILER_CLANG)
#define RX_HINT_UNREACHABLE() \
  __builtin_unreachable()
#elif defined(RX_COMPILER_MSVC)
// Use of |__assume(0)| is a special case that is specifically reserved for
// the purpose of indicating unreachability.
//
// For more information see:
//   https://docs.microsoft.com/en-us/cpp/intrinsics/assume?view=vs-2019
#define RX_HINT_UNREACHABLE() \
  __assume(0)
#else
#define RX_HINT_UNREACHABLE()
#endif // defined(RX_COMPILER_GCC) || defined(RX_COMPILER_CLANG)

#endif // RX_CORE_HINTS_UNREACHABLE_H
