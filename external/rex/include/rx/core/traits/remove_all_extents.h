#ifndef RX_CORE_TRAITS_REMOVE_ALL_EXTENTS_H
#define RX_CORE_TRAITS_REMOVE_ALL_EXTENTS_H
#include "rx/core/types.h" // rx_size
#include "rx/core/traits/type_identity.h"

namespace rx::traits {

namespace detail {
  template<typename T>
  struct remove_all_extents : traits::type_identity<T> {};

  template<typename T>
  struct remove_all_extents<T[]> : traits::type_identity<typename remove_all_extents<T>::type> {};

  template<typename T, rx_size E>
  struct remove_all_extents<T[E]> : traits::type_identity<typename remove_all_extents<T>::type> {};
} // namespace detail

template<typename T>
using remove_all_extents = typename detail::remove_all_extents<T>::type;

} // namespace rx::traits

#endif // RX_CORE_TRAITS_REMOVE_ALL_EXTENTS_H
