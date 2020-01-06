#ifndef RX_CORE_TRAITS_IS_INTEGRAL_H
#define RX_CORE_TRAITS_IS_INTEGRAL_H
#include "rx/core/traits/is_same.h"
#include "rx/core/traits/remove_cv.h"

namespace rx::traits {

namespace detail {
  template<typename T>
  inline constexpr const bool is_integral{
       traits::is_same<T, bool>
    || traits::is_same<T, char>
    || traits::is_same<T, signed char>
    || traits::is_same<T, unsigned char>
    || traits::is_same<T, short>
    || traits::is_same<T, unsigned short>
    || traits::is_same<T, int>
    || traits::is_same<T, unsigned int>
    || traits::is_same<T, long>
    || traits::is_same<T, unsigned long>
    || traits::is_same<T, long long>
    || traits::is_same<T, unsigned long long>
  };
} // namespace detail

template<typename T>
inline constexpr bool is_integral{detail::is_integral<remove_cv<T>>};

} // namespace rx::traits

#endif // RX_CORE_TRAITS_IS_INTEGRAL_H