#include "rx/core/math/sqrt.h"
#include "rx/core/math/shape.h"

#if defined(RX_COMPILER_MSVC)
#pragma warning(disable: 4723) // potential divide by 0
#endif

namespace rx::math {

static constexpr const rx_f32 k_tiny{1.0e-30};

rx_f32 sqrt(rx_f32 _x) {
  const auto sign{static_cast<rx_s32>(0x80000000)};
  rx_s32 ix{shape{_x}.as_s32};

  if ((ix & 0x7f800000) == 0x7f000000) {
    // sqrt(NaN) = NaN
    // sqrt(+inf) = +inf
    // sqrt(-inf) = sNaN
    return _x * _x + _x;
  }

  if (ix <= 0) {
    if ((ix & ~sign) == 0) {
      // sqrt(+-0) => +-0
      return _x;
    }

    if (ix < 0) {
      // sqrt(-ve) => sNaN
      return (_x - _x) / (_x - _x);
    }
  }

  // normalize _x
  rx_s32 m{ix >> 23};
  if (m == 0) {
    // subnormal _x
    rx_s32 i{0};
    for (; (ix & 0x00800000) == 0; i++) {
      ix <<= 1;
    }

    m -= i - 1;
  }

  // unbias exponent
  m -= 127;

  ix = (ix & 0x007fffff) | 0x00800000;
  if (m & 1) {
    // when odd, double to make event
    ix += ix;
  }
  m >>= 1;

  // generate sqrt(_x) bit by bit
  ix += ix;

  rx_s32 q{0}; // q = sqrt(_x)
  rx_s32 s{0};

  // moving bit from right to left
  rx_u32 r{0x01000000};

  while (r != 0) {
    const auto t{static_cast<rx_s32>(s + r)};
    if (t <= ix) {
      s = t + r;
      ix -= t;
      q += r;
    }
    ix += ix;
    r >>= 1;
  }

  // use floating-point add to find rounding direction
  if (ix != 0) {
    // raise inexact flag
    rx_f32 z{1.0f - k_tiny};

    // search for direction
    if (z >= 1.0f) {
      z = 1.0f + k_tiny;
      if (z > 1.0f) {
        q += 2;
      } else {
        q += q & 1;
      }
    }
  }

  ix = (q >> 1) + 0x3f000000;
  ix += static_cast<rx_u32>(m) << 23;

  return shape{ix}.as_f32;
}

} // namespace rx::math
