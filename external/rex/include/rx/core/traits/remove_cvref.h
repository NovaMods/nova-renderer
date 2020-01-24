#ifndef RX_CORE_TRAITS_REMOVE_CVREF_H
#define RX_CORE_TRAITS_REMOVE_CVREF_H
#include "rx/core/traits/remove_cv.h"
#include "rx/core/traits/remove_reference.h"

namespace rx::traits {

template<typename T>
using remove_cvref = remove_cv<remove_reference<T>>;

} // namespace rx::traits


#endif // RX_CORE_TRAITS_REMOVE_CVREF_H
