#ifndef RX_CORE_TRAITS_REMOVE_POINTER_H
#define RX_CORE_TRAITS_REMOVE_POINTER_H
#include "rx/core/traits/type_identity.h"

namespace rx::traits {

namespace detail {
  template<typename T>
  struct remove_pointer : traits::type_identity<T> {};

  template<typename T>
  struct remove_pointer<T*> : traits::type_identity<T> {};

  template<typename T>
  struct remove_pointer<T* const> : traits::type_identity<T> {};

  template<typename T>
  struct remove_pointer<T* volatile> : traits::type_identity<T> {};

  template<typename T>
  struct remove_pointer<T* const volatile> : traits::type_identity<T> {};
} // namespace detail

template<typename T>
using remove_pointer = typename detail::remove_pointer<T>::type;

} // namespace rx::traits

#endif // RX_CORE_TRAITS_REMOVE_POINTER_H
