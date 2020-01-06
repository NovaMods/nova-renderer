#ifndef RX_CORE_TRAITS_IS_VOID_H
#define RX_CORE_TRAITS_IS_VOID_H
#include "rx/core/traits/is_same.h"
#include "rx/core/traits/remove_cv.h"

namespace rx::traits {

template<typename T>
inline constexpr const bool is_void{is_same<void, remove_cv<T>>};

} // namespace rx::traits


#endif // RX_CORE_TRAITS_IS_VOID_H
