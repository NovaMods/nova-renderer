#ifndef RX_CORE_UTILITY_SWAP_H
#define RX_CORE_UTILITY_SWAP_H
#include "rx/core/utility/move.h"

namespace rx::utility {

template<typename T>
inline void swap(T& lhs_, T& rhs_) {
  T tmp{utility::move(lhs_)};
  lhs_ = utility::move(rhs_);
  rhs_ = utility::move(tmp);
}

} // namespace rx::utility

#endif // RX_CORE_UTILITY_SWAP_H
