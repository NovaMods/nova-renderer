#ifndef RX_CORE_MATH_SHAPE_H
#define RX_CORE_MATH_SHAPE_H
#include "rx/core/types.h"

namespace rx::math {

template<typename T>
union shape;

template<>
union shape<rx_f32> {
  constexpr shape(rx_f32 _f);
  constexpr shape(rx_u32 _u);
  constexpr shape(rx_s32 _s);
  rx_f32 as_f32;
  rx_u32 as_u32;
  rx_s32 as_s32;
};

inline constexpr shape<rx_f32>::shape(rx_f32 _f)
  : as_f32{_f}
{
}

inline constexpr shape<rx_f32>::shape(rx_u32 _u)
  : as_u32{_u}
{
}

inline constexpr shape<rx_f32>::shape(rx_s32 _s)
  : as_s32{_s}
{
}

shape(rx_f32) -> shape<rx_f32>;
shape(rx_u32) -> shape<rx_f32>;
shape(rx_s32) -> shape<rx_f32>;

template<>
union shape<rx_f64> {
  constexpr shape(rx_f64 _f);
  constexpr shape(rx_u64 _u);
  constexpr shape(rx_s64 _s);
  rx_f64 as_f64;
  rx_u64 as_u64;
  rx_s64 as_s64;
};

inline constexpr shape<rx_f64>::shape(rx_f64 _f)
  : as_f64{_f}
{
}

inline constexpr shape<rx_f64>::shape(rx_u64 _u)
  : as_u64{_u}
{
}

inline constexpr shape<rx_f64>::shape(rx_s64 _s)
  : as_s64{_s}
{
}

shape(rx_f64) -> shape<rx_f64>;
shape(rx_u64) -> shape<rx_f64>;
shape(rx_s64) -> shape<rx_f64>;

} // namespace rx::math

#endif // RX_CORE_MATH_SHAPE_H