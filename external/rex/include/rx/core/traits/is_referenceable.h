#ifndef RX_CORE_TRAITS_IS_REFERENCEABLE_H
#define RX_CORE_TRAITS_IS_REFERENCEABLE_H
#include "rx/core/traits/is_same.h"

namespace rx::traits {

namespace detail {
  struct is_referenceable {
    template<typename T> static T& test(int);
    template<typename T> static short test(...);
  };
} // namespace detail

template<typename T>
inline constexpr const bool is_referenceable{
  !is_same<decltype(detail::is_referenceable::test<T>(0)), short>
};

} // namespace rx::traits

#endif
