#ifndef RX_CORE_TRAITS_IS_ENUM_H
#define RX_CORE_TRAITS_IS_ENUM_H
#include "rx/core/config.h"

namespace rx::traits {

#if __has_feature(is_enum) || defined(RX_COMPILER_GCC) || defined(RX_COMPILER_MSVC)
template<typename T>
inline constexpr bool is_enum = __is_enum(T);
#else
#error "missing __is_enum compiler intrinsic"
#endif

} // namespace rx::traits

#endif // RX_CORE_TRAITS_IS_ARRAY_H
