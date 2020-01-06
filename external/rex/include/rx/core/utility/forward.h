#ifndef RX_CORE_UTILITY_FORWARD_H
#define RX_CORE_UTILITY_FORWARD_H
#include "rx/core/traits/remove_reference.h"
#include "rx/core/traits/is_lvalue_reference.h"

namespace rx::utility {

template<typename T>
inline constexpr T&& forward(traits::remove_reference<T>& _value) {
  return static_cast<T&&>(_value);
}

template<typename T>
inline constexpr T&& forward(traits::remove_reference<T>&& _value) {
  static_assert(!traits::is_lvalue_reference<T>,
    "cannot forward an rvalue as an lvalue");
  return static_cast<T&&>(_value);
}

} // namespace utility

#endif // RX_CORE_UTILITY_FORWARD_H
