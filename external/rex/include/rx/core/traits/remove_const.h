#ifndef RX_CORE_TRAITS_REMOVE_CONST_H
#define RX_CORE_TRAITS_REMOVE_CONST_H
#include "rx/core/traits/type_identity.h"

namespace rx::traits {

namespace detail {
  template<typename T>
  struct remove_const : traits::type_identity<T> {};

  template<typename T>
  struct remove_const<const T> : traits::type_identity<T> {};
} // namespace detail

template<typename T>
using remove_const = typename detail::remove_const<T>::type;

} // namespace rx::traits

#endif // RX_CORE_TRAITS_REMOVE_CONST_H
