#ifndef RX_CORE_ALGORITHM_MIN_H
#define RX_CORE_ALGORITHM_MIN_H
#include "rx/core/utility/forward.h"

namespace rx::algorithm {

template<typename T>
inline constexpr T min(T _value) {
  return _value;
}

template<typename T, typename... Ts>
inline constexpr T min(T _a, T _b, Ts&&... _args) {
  return min(_a < _b ? _a : _b, utility::forward<Ts>(_args)...);
}

} // namespace rx::algorithm

#endif // RX_CORE_ALGORITHM_MIN_H
