#ifndef RX_CORE_TRAITS_ADD_VOLATILE_H
#define RX_CORE_TRAITS_ADD_VOLATILE_H

namespace rx::traits {

template<typename T>
using add_volatile = volatile T;

} // namespace rx::traits

#endif // RX_CORE_TRAITS_ADD_VOLATILE_H
