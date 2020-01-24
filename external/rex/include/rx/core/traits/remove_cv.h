#ifndef RX_CORE_TRAITS_REMOVE_CV_H
#define RX_CORE_TRAITS_REMOVE_CV_H
#include "rx/core/traits/remove_const.h"
#include "rx/core/traits/remove_volatile.h"

namespace rx::traits {

template<typename T>
using remove_cv = remove_volatile<remove_const<T>>;

} // namespace rx::traits

#endif // RX_CORE_TRAITS_REMOVE_CV_H
