#ifndef RX_CORE_TRAITS_CONDITIONAL_H
#define RX_CORE_TRAITS_CONDITIONAL_H
#include "rx/core/traits/type_identity.h"

namespace rx::traits {

namespace detail {
  template<bool B, typename T, typename F>
  struct conditional : traits::type_identity<T> {};

  template<typename T, typename F>
  struct conditional<false, T, F> : traits::type_identity<F> {};
} // namespace detail

template<bool B, typename T, typename F>
using conditional = typename detail::conditional<B, T, F>::type;

} // namespace rx::traits

#endif // RX_CORE_TRAITS_CONDITIONAL_H
