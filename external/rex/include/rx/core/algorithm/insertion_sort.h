#ifndef RX_CORE_ALGORITHM_INSERTION_SORT_H
#define RX_CORE_ALGORITHM_INSERTION_SORT_H
#include "rx/core/utility/move.h"

namespace rx::algorithm {

//! insertion sort from _start to _end using _compare comparator
template<typename T, typename F>
inline void insertion_sort(T* start_, T* end_, F&& _compare) {
  for (T* item1 = start_ + 1; item1 < end_; item1++) {
    if (_compare(*item1, *(item1 - 1))) {
      T temp{utility::move(*item1)};
      *item1 = utility::move(*(item1 - 1));
      T* item2{item1 - 1};
      for (; item2 > start_ && _compare(temp, *(item2 - 1)); --item2) {
        *item2 = utility::move(*(item2 - 1));
      }
      *item2 = utility::move(temp);
    }
  }
}

} // namespace rx::algorithm

#endif // RX_CORE_ALGORITHM_INSERTION_SORT_H
