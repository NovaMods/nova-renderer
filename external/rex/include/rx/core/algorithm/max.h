#ifndef RX_CORE_ALGORITHM_MAX_H
#define RX_CORE_ALGORITHM_MAX_H
#include "rx/core/utility/forward.h"
#include "rx/core/hints/force_inline.h"

namespace rx::algorithm {

template<typename T>
RX_HINT_FORCE_INLINE constexpr T max(T _a) {
  return _a;
}

template<typename T, typename... Ts>
RX_HINT_FORCE_INLINE constexpr T max(T _a, T _b, Ts&&... _args) {
  return max(_a > _b ? _a : _b, utility::forward<Ts>(_args)...);
}

} // namespace rx::algorithm

#endif // RX_CORE_ALGORITHM_MAX_H
