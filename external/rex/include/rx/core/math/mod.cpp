#include "rx/core/math/mod.h"
#include "rx/core/math/isnan.h"

namespace rx::math {

rx_f32 mod(rx_f32 _x, rx_f32 _y) {
  shape ux{_x};
  shape uy{_y};

  auto ex{static_cast<int>(ux.as_u32 >> 23 & 0xff)};
  auto ey{static_cast<int>(uy.as_u32 >> 23 & 0xff)};

  rx_u32 sx = ux.as_u32 & 0x80000000;
  rx_u32 uxi = ux.as_u32;

  if (uy.as_u32 << 1 == 0 || isnan(_y) || ex == 0xff) {
    return (_x * _y) / (_x * _y);
  }

  if (uxi << 1 <= uy.as_u32 << 1) {
    if (uxi << 1 == uy.as_u32 << 1) {
      return 0 * _x;
    }

    return _x;
  }

  // normalize x and y
  if (!ex) {
    for (rx_u32 i{uxi << 9}; i >> 31 == 0; ex--, i <<= 1);
    uxi <<= -ex + 1;
  } else {
    uxi &= -1u >> 9;
    uxi |= 1u << 23;
  }

  if (!ey) {
    for (rx_u32 i{uy.as_u32 << 9}; i >> 31 == 0; ey--, i <<= 1);
    uy.as_u32 <<= -ey + 1;
  } else {
    uy.as_u32 &= -1u >> 9;
    uy.as_u32 |= 1u << 23;
  }

  // x mod y
  for (; ex > ey; ex--) {
    const rx_u32 i{uxi - uy.as_u32};
    if (i >> 31 == 0) {
      if (i == 0) {
        return 0 * _x;
      }
      uxi = i;
    }
    uxi <<= 1;
  }

  const rx_u32 i{uxi - uy.as_u32};
  if (i >> 31 == 0) {
    if (i == 0) {
      return 0 * _x;
    }
    uxi = i;
  }
  for (; uxi >> 23 == 0; uxi <<= 1, ex--);

  // scale result up
  if (ex > 0) {
    uxi -= 1u << 23;
    uxi |= static_cast<rx_u32>(ex) << 23;
  } else {
    uxi >>= -ex + 1;
  }
  uxi |= sx;
  ux.as_u32 = uxi;

  return ux.as_f32;
}

} // namespace rx::math