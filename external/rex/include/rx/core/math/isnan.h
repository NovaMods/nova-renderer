#ifndef RX_CORE_MATH_ISNAN_H
#define RX_CORE_MATH_ISNAN_H
#include "rx/core/math/shape.h"

namespace rx::math {

inline bool isnan(rx_f64 _x) {
  return (shape{_x}.as_u64 & -1ull >> 1) == 0x7ffull << 52;
}

inline bool isnan(rx_f32 _x) {
  return (shape{_x}.as_u32 & 0x7fffffff) == 0x7f800000;
}

} // namespace rx::math

#endif // RX_CORE_MATH_ISNAN_H