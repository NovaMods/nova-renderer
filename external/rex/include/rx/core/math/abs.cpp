#include "rx/core/math/abs.h"
#include "rx/core/math/shape.h"

namespace rx::math {

rx_f64 abs(rx_f64 _x) {
  shape u{_x};
  u.as_u64 &= -1ull / 2;
  return u.as_f64;
}

rx_f32 abs(rx_f32 _x) {
  shape u{_x};
  u.as_u32 &= 0x7fffffff;
  return u.as_f32;
}

} // namespace rx::math