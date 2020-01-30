#ifndef RX_CORE_TRAITS_IS_RESTRICT_H
#define RX_CORE_TRAITS_IS_RESTRICT_H
#include "rx/core/traits/remove_cv.h"
#include "rx/core/hints/restrict.h"

namespace rx::traits {

namespace detail {
  template<typename T>
  inline constexpr const bool is_restrict{false};

  template<typename T>
  inline constexpr const bool is_restrict<T *RX_HINT_RESTRICT>{true};
} // namespace detail

template<typename T>
inline constexpr const bool is_restrict{detail::is_restrict<remove_cv<T>>};

} // namespace rx::traits

#endif // RX_CORE_TRAITS_IS_RESTRICT_H
