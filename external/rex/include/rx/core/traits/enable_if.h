#ifndef RX_CORE_TRAITS_ENABLE_IF_H
#define RX_CORE_TRAITS_ENABLE_IF_H
#include "rx/core/traits/type_identity.h"

namespace rx::traits {

namespace detail {
  template<bool B, typename T>
  struct enable_if {};

  template<typename T>
  struct enable_if<true, T> : traits::type_identity<T> {};
} // namespace detail

template<bool B, typename T = void>
using enable_if = typename detail::enable_if<B, T>::type;

} // namespace rx::traits

#endif // RX_CORE_TRAITS_ENABLE_IF_H