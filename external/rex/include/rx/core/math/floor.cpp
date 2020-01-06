#include <float.h> // {DBL,LDBL}_EPSILON

#include "rx/core/math/floor.h"
#include "rx/core/math/shape.h"
#include "rx/core/math/force_eval.h"

#include "rx/core/config.h"

#if RX_FLOAT_EVAL_METHOD == 0 || RX_FLOAT_EVAL_METHOD == 1
static constexpr const rx_f64_eval k_to_int{1 / DBL_EPSILON};
#else
static constexpr const rx_f64_eval k_to_int{1 / LDBL_EPSILON};
#endif

namespace rx::math {

rx_f64 floor(rx_f64 _x) {
  shape u{_x};

  const auto e{static_cast<rx_s32>(u.as_u64 >> 52 & 0x7ff)};

  if (e >= 0x3ff+52 || _x == 0) {
    return _x;
  }

  // y = int(x) - x, where int(x) is an integer neighbor of x
  rx_f64_eval y;
  if (u.as_u64 >> 63) {
    y = _x - k_to_int + k_to_int - _x;
  } else {
    y = _x + k_to_int - k_to_int - _x;
  }

  // special case because of non-nearest rounding modes
  if (e <= 0x3ff-1) {
    force_eval_f64(y);
    return u.as_u64 >> 63 ? -1 : 0;
  }

  if (y > 0) {
    return _x + y - 1;
  }

  return _x + y;
}

rx_f32 floor(rx_f32 _x) {
  shape u{_x};

  const auto e{static_cast<int>(u.as_u32 >> 23 & 0xff) - 0x7f};

  if (e >= 23) {
    return _x;
  }

  if (e >= 0) {
    const auto m{static_cast<rx_u32>(0x007fffff >> e)};
    if ((u.as_u32 & m) == 0) {
      return _x;
    }

    if (u.as_u32 >> 31) {
      u.as_u32 += m;
    }

    u.as_u32 &= ~m;
  } else {
    if (u.as_u32 >> 31 == 0) {
      u.as_u32 = 0;
    } else if (u.as_u32 << 1) {
      u.as_f32 = -1.0;
    }
  }

  return u.as_f32;
}

} // namespace rx::math

