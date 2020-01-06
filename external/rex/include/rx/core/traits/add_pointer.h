#ifndef RX_CORE_TRAITS_ADD_POINTER_H
#define RX_CORE_TRAITS_ADD_POINTER_H
#include "rx/core/traits/remove_reference.h"
#include "rx/core/traits/is_function.h"
#include "rx/core/traits/type_identity.h"

namespace rx::traits {

namespace detail {
  template<typename T, bool F = false>
  struct add_pointer : traits::type_identity<traits::remove_reference<T>*> {};

  template<typename T>
  struct add_pointer<T, true> : traits::type_identity<T> {};

  template<typename T, typename... Ts>
  struct add_pointer<T(Ts...), true> : traits::type_identity<T(*)(Ts...)> {};

  template<typename T, typename... Ts>
  struct add_pointer<T(Ts..., ...), true> : traits::type_identity<T(*)(Ts..., ...)> {};
} // namespace detail

template<typename T>
using add_pointer = typename detail::add_pointer<T, is_function<T>>::type;

} // namespace rx::traits

#endif // RX_CORE_TRAITS_ADD_POINTER_H
