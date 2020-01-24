#include "rx/core/math/pow.h"
#include "rx/core/math/shape.h"

namespace rx::math {

rx_f32 pow(rx_f32 _x, rx_f32 _y) {
  // TODO(dweiler): implement a valid pow
  shape u{static_cast<rx_f64>(_x)};
  const auto t1{static_cast<rx_s32>(u.as_s64 >> 32)};
  const auto t2{static_cast<rx_s32>(_y * (t1 - 1072632447) + 1072632447)};
  return static_cast<rx_f32>(shape{static_cast<rx_s64>(t2) << 32}.as_f64);
}

} // namespace rx::math
