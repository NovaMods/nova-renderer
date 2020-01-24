#ifndef RX_CORE_ALGORITHM_MAX_H
#define RX_CORE_ALGORITHM_MAX_H
#include "rx/core/utility/forward.h"

namespace rx::algorithm {

template<typename T>
inline constexpr T max(T _a) {
  return _a;
}

template<typename T, typename... Ts>
inline constexpr T max(T _a, T _b, Ts&&... _args) {
  return max(_a > _b ? _a : _b, utility::forward<Ts>(_args)...);
}

} // namespace rx::algorithm

#endif // RX_CORE_ALGORITHM_MAX_H
