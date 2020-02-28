#ifndef RX_CORE_HINTS_ASSUME_ALIGNED_H
#define RX_CORE_HINTS_ASSUME_ALIGNED_H
#include "rx/core/config.h" // RX_COMPILER_{GCC,CLANG,MSVC}

// # Assume alignment
//
// Hint that a given pointer is guaranteed to be aligned on a certain alignment
// to allow the compiler to make less-safe optimizations with regards to load
// and store instructions.
#if defined(RX_COMPILER_GCC) || defined(RX_COMPILER_CLANG)
#include "rx/core/traits/conditional.h"
#include "rx/core/traits/is_restrict.h"
// Use |__builtin_assume_aligned| to hint to the compiler that a given pointer
// is aligned, unfortunately the return value of the intrinsic must be assigned
// to a variable to mark that pointer aligned, here we just overwrite the
// pointer given by |_pointer| to permit that.
//
// Some pointers may be marked as restrict with RX_HINT_RESTRICT so some
// meta-programming is required to cast it correctly so that the restrict
// qualifier isn't lost in the hint.
//
// The return type of |__builtin_assume_aligned| is always a void* so we opt
// to take the address of the pointer as an lvalue and cast to void**, which
// is then dereferenced to assign the pointer the result of the intrinsic.
#define RX_HINT_ASSUME_ALIGNED(_pointer, _alignment) \
    *(rx::traits::conditional<rx::traits::is_restrict<decltype(_pointer)>, \
       void *RX_HINT_RESTRICT *, void**>)(&(_pointer)) \
      = __builtin_assume_aligned((_pointer), (_alignment))
#elif defined(RX_COMPILER_MSVC)
// Use |__assume| intrinsic on MSVC. It should offer similar performance
// opportunities.
#define RX_HINT_ASSUME_ALIGNED(_pointer, _alignment) \
  __assume(reinterpret_cast<rx_uintptr>(_pointer) % (_alignment) == 0)
#else
#define RX_HINT_ASSUME_ALIGNED(_pointer, _alignment) \
  RX_ASSERT(reinterpret_cast<rx_uintptr>(_pointer) % (_alignment) == 0,
    "not aligned by %zu", (_alignment))
#endif

#endif // RX_CORE_HINTS_ASSUME_ALIGNED_H
