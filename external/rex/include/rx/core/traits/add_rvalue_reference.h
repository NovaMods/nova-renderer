#ifndef RX_CORE_TRAITS_ADD_RVALUE_REFERENCE_H
#define RX_CORE_TRAITS_ADD_RVALUE_REFERENCE_H
#include "rx/core/traits/is_referenceable.h"
#include "rx/core/traits/type_identity.h"

namespace rx::traits {

namespace detail {
  template<typename T, bool = traits::is_referenceable<T>>
  struct add_rvalue_reference : traits::type_identity<T> {};

  template<typename T>
  struct add_rvalue_reference<T, true> : traits::type_identity<T&&> {};
} // namespace detail

template<typename T>
using add_rvalue_reference = typename detail::add_rvalue_reference<T>::type;

} // namespace rx::traits

#endif // RX_CORE_TRAITS_ADD_RVALUE_REFERENCE_H
