#ifndef RX_CORE_UTILITY_CONSTRUCT_H
#define RX_CORE_UTILITY_CONSTRUCT_H
#include "rx/core/types.h" // rx_size
#include "rx/core/utility/forward.h" // utility::forward
#include "rx/core/hints/force_inline.h"

struct rx_placement_new {};

RX_HINT_FORCE_INLINE void* operator new(rx_size, void* _data, rx_placement_new) {
  return _data;
}

namespace rx::utility {

template<typename T, typename... Ts>
RX_HINT_FORCE_INLINE T* construct(void* _data, Ts&&... _args) {
  return new (_data, rx_placement_new{}) T{utility::forward<Ts>(_args)...};
}

} // namespace rx::utility

#endif // RX_CORE_UTILITY_CONSTRUCT_H
