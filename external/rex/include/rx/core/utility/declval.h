#ifndef RX_CORE_UTILITY_DECLVAL_H
#define RX_CORE_UTILITY_DECLVAL_H
#include "rx/core/traits/add_rvalue_reference.h" // traits::add_rvalue_reference

namespace rx::utility {
  
template<typename T>
traits::add_rvalue_reference<T> declval();

} // namespace rx::utility

#endif // RX_CORE_UTILITY_DECLVAL_H