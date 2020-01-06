#ifndef RX_CORE_TRAITS_REMOVE_VOLATILE_H
#define RX_CORE_TRAITS_REMOVE_VOLATILE_H
#include "rx/core/traits/type_identity.h"

namespace rx::traits {

namespace detail {
  template<typename T>
  struct remove_volatile : traits::type_identity<T> {};

  template<typename T>
  struct remove_volatile<volatile T> : traits::type_identity<T> {};
} // namespace detail

template<typename T>
using remove_volatile = typename detail::remove_volatile<T>::type;

} // namespace rx::traits

#endif // RX_CORE_TRAITS_REMOVE_VOLATILE_H
