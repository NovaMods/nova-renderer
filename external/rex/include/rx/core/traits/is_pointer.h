#ifndef RX_CORE_TRAITS_IS_POINTER_H
#define RX_CORE_TRAITS_IS_POINTER_H
#include "rx/core/traits/remove_cv.h"

namespace rx::traits {

namespace detail {
  template<typename T>
  inline constexpr const bool is_pointer{false};

  template<typename T>
  inline constexpr const bool is_pointer<T*>{true};
} // namespace detail

template<typename T>
inline constexpr const bool is_pointer{detail::is_pointer<remove_cv<T>>};

} // namespace rx::traits

#endif // RX_CORE_TRAITS_IS_POINTER_H
