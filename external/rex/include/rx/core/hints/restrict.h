#ifndef RX_CORE_HINTS_RESTRICT_H
#define RX_CORE_HINTS_RESTRICT_H
#include "rx/core/config.h" // RX_COMPILER_{GCC,CLANG,MSVC}

// # Restrict
//
// Hint that a given pointer does not alias the memory of another pointer.
//
// When the compiler cannot tell that two pointers may potentially overlap
// the memory they're reading and writing to, the compiler has to be pessimistic
// about loads and stores since they may alias the same memory.
//
// This hint prevents those additional loads and stores from being generated
// by indicating that such accesses will never overlap.
#if defined(RX_COMPILER_GCC) || defined(RX_COMPILER_CLANG)
#define RX_HINT_RESTRICT __restrict__
#elif defined(RX_COMPILER_MSVC)
#define RX_HINT_RESTRICT __restrict
#else
#define RX_HINT_RESTRICT
#endif

#endif // RX_CORE_HINTS_RESTRICT_H
