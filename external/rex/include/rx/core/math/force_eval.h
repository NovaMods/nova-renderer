#ifndef RX_CORE_MATH_FORCE_EVAL_H
#define RX_CORE_MATH_FORCE_EVAL_H
#include "rx/core/types.h"

inline void force_eval_f32(rx_f32 _x) {
  [[maybe_unused]] volatile rx_f32 y;
  y = _x;
}

inline void force_eval_f64(rx_f64 _x) {
  [[maybe_unused]] volatile rx_f64 y;
  y = _x;
}

#endif // RX_CORE_MATH_FORCE_EVAL_H