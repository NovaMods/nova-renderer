#ifndef RX_CORE_HINTS_MAY_ALIAS_H
#define RX_CORE_HINTS_MAY_ALIAS_H
#include "rx/core/config.h"

#if defined(RX_COMPILER_GCC) || defined(RX_COMPILER_CLANG)
#define RX_HINT_MAY_ALIAS __attribute__((__may_alias__))
#else
#define RX_HINT_MAY_ALIAS
#endif

#endif // RX_CORE_HINTS_MAY_ALIAS_H
