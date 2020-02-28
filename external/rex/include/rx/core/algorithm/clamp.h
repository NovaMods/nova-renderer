#ifndef RX_CORE_ALGORITHM_CLAMP_H
#define RX_CORE_ALGORITHM_CLAMP_H
#include "rx/core/hints/force_inline.h"

namespace rx::algorithm {

//! clamp value to given range
template<typename T>
RX_HINT_FORCE_INLINE constexpr T clamp(T _value, T _min, T _max) {
  return _value < _min ? _min : (_value > _max ? _max : _value);
}

} // namespace rx::algorithm

#endif // RX_CORE_ALGORITHM_CLAMP_H
