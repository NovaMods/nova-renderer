#ifndef RX_CORE_TRAITS_RETURN_TYPE_H
#define RX_CORE_TRAITS_RETURN_TYPE_H
#include "rx/core/traits/type_identity.h"

namespace rx::traits {

namespace detail {
  template<typename T>
  struct return_type : return_type<decltype(&T::operator())> {};
  template<typename R, typename... Ts>
  struct return_type<R (Ts...)> : traits::type_identity<R> {};
  template<typename R, typename... Ts>
  struct return_type<R (*)(Ts...)> : traits::type_identity<R> {};
  template<typename T, typename R, typename... Ts>
  struct return_type<R (T::*)(Ts...)> : traits::type_identity<R> {};
  template<typename T, typename R, typename... Ts>
  struct return_type<R (T::*)(Ts...) const> : traits::type_identity<R> {};
  template<typename T, typename R, typename... Ts>
  struct return_type<R (T::*)(Ts...) volatile> : traits::type_identity<R> {};
  template<typename T, typename R, typename... Ts>
  struct return_type<R (T::*)(Ts...) const volatile> : traits::type_identity<R> {};
  template<typename T, typename R, typename... Ts>
  struct return_type<R (T::*)(Ts...) &> : traits::type_identity<R> {};
  template<typename T, typename R, typename... Ts>
  struct return_type<R (T::*)(Ts...) &&> : traits::type_identity<R> {};
  template<typename T, typename R, typename... Ts>
  struct return_type<R (T::*)(Ts...) const &> : traits::type_identity<R> {};
  template<typename T, typename R, typename... Ts>
  struct return_type<R (T::*)(Ts...) volatile &> : traits::type_identity<R> {};
  template<typename T, typename R, typename... Ts>
  struct return_type<R (T::*)(Ts...) const volatile &> : traits::type_identity<R> {};
} // namespace detail

template<typename T>
using return_type = typename detail::return_type<T>::type;

} // namespace rx::traits

#endif // RX_CORE_TRAITS_RETURN_TYPE_H
