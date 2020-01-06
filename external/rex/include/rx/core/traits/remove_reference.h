#ifndef RX_CORE_TRAITS_REMOVE_REFERENCE_H
#define RX_CORE_TRAITS_REMOVE_REFERENCE_H
#include "rx/core/traits/type_identity.h"

namespace rx::traits {

namespace detail {
  template<typename T>
  struct remove_reference : traits::type_identity<T> {};

  template<typename T>
  struct remove_reference<T&> : traits::type_identity<T> {};

  template<typename T>
  struct remove_reference<T&&> : traits::type_identity<T> {};
} // namespace detail

template<typename T>
using remove_reference = typename detail::remove_reference<T>::type;

} // namespace rx::traits

#endif // RX_CORE_TRAITS_REMOVE_REFERENCE_H
