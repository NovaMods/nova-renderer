#include "rx/core/math/half.h" // half
#include "rx/core/math/shape.h" // shape

#include "rx/core/global.h"

namespace rx::math {

static constexpr const rx_u32 k_magic{113 << 23};
static constexpr const rx_u32 k_shift_exp{0x7C00 << 13}; // exp mask after shift
static constexpr shape<rx_f32> k_magic_bits{k_magic};

struct half_lut {
  half_lut() {
    for (int i{0}, e{0}; i < 256; i++) {
      e = i - 127;
      if (e < -24) {
        base[i|0x000] = 0x0000;
        base[i|0x100] = 0x8000;
        shift[i|0x000] = 24;
        shift[i|0x100] = 24;
      } else if (e < -14) {
        base[i|0x000] = 0x0400 >> (-e-14);
        base[i|0x100] = (0x0400 >> (-e-14)) | 0x8000;
        shift[i|0x000] = -e-1;
        shift[i|0x100] = -e-1;
      } else if (e <= 15) {
        base[i|0x000] = (e+15) << 10;
        base[i|0x100] = ((e+15) << 10) | 0x8000;
        shift[i|0x000] = 13;
        shift[i|0x100] = 13;
      } else if (e < 128) {
        base[i|0x000] = 0x7C00;
        base[i|0x100] = 0xFC00;
        shift[i|0x000] = 24;
        shift[i|0x100] = 24;
      } else {
        base[i|0x000] = 0x7C00;
        base[i|0x100] = 0xFC00;
        shift[i|0x000] = 13;
        shift[i|0x100] = 13;
      }
    }
  }

  rx_u32 base[512];
  rx_u8 shift[512];
};

static const RX_GLOBAL<half_lut> g_table{"system", "half"};

half half::to_half(rx_f32 _f) {
  const shape u{_f};
  return half(static_cast<rx_u16>(g_table->base[(u.as_u32 >> 23) & 0x1FF] +
    ((u.as_u32 & 0x007FFFFF) >> g_table->shift[(u.as_u32 >> 23) & 0x1FF])));
}

rx_f32 half::to_f32() const {
  shape out{static_cast<rx_u32>((m_bits & 0x7FFF) << 13)}; // exp/mantissa
  const auto exp{k_shift_exp & out.as_u32}; // exp
  out.as_u32 += (127 - 15) << 23; // adjust exp
  if (exp == k_shift_exp) {
    out.as_u32 += (128 - 16) << 23; // adjust for inf/nan
  } else if (exp == 0) {
    out.as_u32 += 1 << 23; // adjust for zero/denorm
    out.as_f32 -= k_magic_bits.as_f32; // renormalize
  }
  out.as_u32 |= (m_bits & 0x8000) << 16; // sign bit
  return out.as_f32;
}

} // namespace rx::math
