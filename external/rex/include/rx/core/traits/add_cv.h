#ifndef RX_CORE_TRAITS_ADD_CV_H
#define RX_CORE_TRAITS_ADD_CV_H

namespace rx::traits {

template<typename T>
using add_cv = const volatile T;

} // namespace rx::traits

#endif // RX_CORE_TRAITS_ADD_CV_H
