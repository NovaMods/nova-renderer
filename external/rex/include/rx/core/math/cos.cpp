#define _USE_MATH_DEFINES
#include <math.h> // M_PI_2

#include "rx/core/math/cos.h"
#include "rx/core/math/sqrt.h"
#include "rx/core/math/shape.h"
#include "rx/core/math/force_eval.h"

#include "rx/core/hints/unreachable.h"

#if defined(RX_COMPILER_MSVC)
#pragma warning(disable: 4723) // potential divide by 0
#endif

namespace rx::math {

// |cos(x) - c(x)| < 2**-34.1 (~[-5.37e-11, 5.295e-11])
static constexpr const rx_f64 k_c0{-0x1ffffffd0c5e81.0p-54}; // -0.499999997251031003120
static constexpr const rx_f64 k_c1{ 0x155553e1053a42.0p-57}; //  0.0416666233237390631894
static constexpr const rx_f64 k_c2{-0x16c087e80f1e27.0p-62}; // -0.00138867637746099294692
static constexpr const rx_f64 k_c3{ 0x199342e0ee5069.0p-68}; //  0.0000243904487962774090654

rx_f32 sindf(rx_f64 _x); // sin.cpp
rx_f32 cosdf(rx_f64 _x) {
  const rx_f64_eval z{_x*_x};
  const rx_f64_eval w{z*z};
  const rx_f64_eval r{k_c2+z*k_c3};
  return static_cast<rx_f32>(((1.0+z*k_c0) + w*k_c1) + (w*z)*r);
}

// small multiplies of pi/2 rounded to double precision
static constexpr const rx_f64 k_c1_pi_2{1 * M_PI_2};
static constexpr const rx_f64 k_c2_pi_2{2 * M_PI_2};
static constexpr const rx_f64 k_c3_pi_2{3 * M_PI_2};
static constexpr const rx_f64 k_c4_pi_2{4 * M_PI_2};

rx_s32 rempio2(rx_f32 _x, rx_f64& y_); // sin.cpp

rx_f32 cos(rx_f32 _x) {
  rx_u32 ix{shape{_x}.as_u32};
  const rx_u32 sign{ix >> 31};

  ix &= 0x7fffffff;

  // |_x| ~<= pi/4
  if (ix <= 0x3f490fda) {
    // |_x| < 2**-12
    if (ix < 0x39800000) {
      // raise inexact if _x != 0
      force_eval_f32(_x + 0x1p120f);
      return 1.0f;
    }
    return cosdf(_x);
  }

  // |_x| ~<= 5*pi/4
  if (ix <= 0x407b53d1) {
    // |_x| ~> 3*pi/4
    if (ix > 0x4016cbe3) {
      return -cosdf(sign ? _x+k_c2_pi_2 : _x-k_c2_pi_2);
    } else {
      if (sign) {
        return sindf(_x + k_c1_pi_2);
      } else {
        return sindf(k_c1_pi_2 - _x);
      }
    }
  }

  // |_x| ~<= 9*pi/4
  if (ix <= 0x40e231d5) {
    // |_x| ~> 7*pi/4
    if (ix > 0x40afeddf) {
      return cosdf(sign ? _x+k_c4_pi_2 : _x-k_c4_pi_2);
    } else {
      if (sign) {
        return sindf(-_x - k_c3_pi_2);
      } else {
        return sindf(_x - k_c3_pi_2);
      }
    }
  }

  // cos(+inf) = NaN
  // cos(-inf) = NaN
  // cos(NaN) = NaN
  if (ix >= 0x7f800000) {
    return _x - _x;
  }

  rx_f64 y;
  switch (rempio2(_x, y) & 3) {
  case 0:
    return cosdf(y);
  case 1:
    return sindf(-y);
  case 2:
    return -cosdf(y);
  default:
    return sindf(y);
  }

  RX_HINT_UNREACHABLE();
}

static constexpr const rx_f32 k_pi_2_hi{1.5707962513e+00}; // 0x3fc90fda
static constexpr const rx_f32 k_pi_2_lo{7.5497894159e-08}; // 0x33a22168
static constexpr const rx_f32 k_p_s0{1.6666586697e-01};
static constexpr const rx_f32 k_p_s1{-4.2743422091e-02};
static constexpr const rx_f32 k_p_s2{-8.6563630030e-03};
static constexpr const rx_f32 k_q_s1{-7.0662963390e-01};

static rx_f32 R(rx_f32 _z) {
  const rx_f32_eval p{_z*(k_p_s0+_z*(k_p_s1+_z*k_p_s2))};
  const rx_f32_eval q{1.0f+_z*k_q_s1};
  return static_cast<rx_f32>(p/q);
}

rx_f32 acos(rx_f32 _x) {
  rx_u32 hx{shape{_x}.as_u32};
  rx_u32 ix = hx & 0x7fffffff;

  // |_x| >= 1 | NaN
  if (ix >= 0x3f800000) {
    if (ix == 0x3f000000) {
      if (hx >> 31) {
        return 2*k_pi_2_hi + 0x1p-120f;
      }
      return 0;
    }
    return 0 / (_x - _x);
  }

  // |_x| < 0.5
  if (ix < 0x3f000000) {
    // |_x| < 2**-26
    if (ix <= 0x32800000) {
      return k_pi_2_hi + 0x1p-120f;
    }
    return k_pi_2_hi - (_x - (k_pi_2_lo - _x * R(_x*_x)));
  }

  // x < -0.5
  if (hx >> 31) {
    const rx_f32 z{(1+_x)*0.5f};
    const rx_f32 s{sqrt(z)};
    const rx_f32 w{R(z)*s-k_pi_2_lo};
    return 2*(k_pi_2_lo - (s+w));
  }

  // x > 0.5
  const rx_f32 z{(1-_x)*0.5f};
  const rx_f32 s{sqrt(z)};
  const rx_f32 df{shape{shape{s}.as_u32 & 0xfffff000}.as_f32};
  const rx_f32 c{(z-df*df)/(s+df)};
  const rx_f32 w{R(z)*s+c};
  return 2*(df+w);
}

} // namespace rx::math
