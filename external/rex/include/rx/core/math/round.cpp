#include <float.h> // FLT_EPSILON, DBL_EPSILON, LDBL_EPSILON

#include "rx/core/math/round.h"
#include "rx/core/math/shape.h"
#include "rx/core/math/force_eval.h"

namespace rx::math {

#if RX_FLOAT_EVAL_METHOD == 0
static constexpr const auto k_eps{FLT_EPSILON};
#elif RX_FLOAT_EVAL_METHOD == 1
static constexpr const auto k_eps{DBL_EPSILON};
#elif RX_FLOAT_EVAL_METHOD == 2
static constexpr const auto k_eps{LDBL_EPSILON};
#endif

static constexpr const rx_f32_eval k_to_int{1 / k_eps};

rx_f32 round(rx_f32 _x) {
  const shape u{_x};
  const rx_s32 e = u.as_u32 >> 23 & 0xff;

  if (e >= 0x7f+23) {
    return _x;
  }

  if (u.as_u32 >> 31) {
    _x = -_x;
  }

  if (e < 0x7f-1) {
    force_eval_f32(_x + k_to_int);
    return 0 * u.as_f32;
  }

  rx_f32_eval y{_x + k_to_int - k_to_int - _x};
  if (y > 0.5f) {
    y = y + _x - 1;
  } else if (y <= -0.5f) {
    y = y + _x + 1;
  } else {
    y = y + _x;
  }

  if (u.as_u32 >> 31) {
    y = -y;
  }

  return y;
}

} // namespace rx::math
