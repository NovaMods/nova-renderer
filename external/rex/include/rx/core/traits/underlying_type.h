#ifndef RX_CORE_TRAITS_UNDERLYING_TYPE_H
#define RX_CORE_TRAITS_UNDERLYING_TYPE_H
#include "rx/core/traits/is_enum.h"

namespace rx::traits {

namespace detail {
  template<typename T, bool = is_enum<T>>
  struct underlying_type;

  template<typename T>
  struct underlying_type<T, false> {};

  template<typename T>
  struct underlying_type<T, true> {
    using type = __underlying_type(T);
  };
} // namespace detail

template<typename T>
using underlying_type = typename detail::underlying_type<T>::type;

} // namespace rx::traits

#endif // RX_CORE_UNDERLYING_TYPE_H
