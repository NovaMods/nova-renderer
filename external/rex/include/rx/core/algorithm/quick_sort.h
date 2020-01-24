#ifndef RX_CORE_ALGORITHM_QUICK_SORT_H
#define RX_CORE_ALGORITHM_QUICK_SORT_H
#include "rx/core/algorithm/insertion_sort.h"

#include "rx/core/utility/forward.h"
#include "rx/core/utility/swap.h"

namespace rx::algorithm {

template<typename T, typename F>
void quick_sort(T* start_, T* end_, F&& _compare) {
  while (end_ - start_ > 10) {
    T* middle{start_ + (end_ - start_) / 2};
    T* item1{start_+1};
    T* item2{end_-2};
    T pivot;

    if (_compare(*start_, *middle)) {
      // start < middle
      if (_compare(*(end_ - 1), *start_)) {
        // end < start < middle
        pivot = utility::move(*start_);
        *start_ = utility::move(*(end_ - 1));
        *(end_ - 1) = utility::move(*middle);
      } else if (_compare(*(end_ - 1), *middle)) {
        // start <= end < middle
        pivot = utility::move(*(end_ - 1));
        *(end_ - 1) = utility::move(*middle);
      } else {
        pivot = utility::move(*middle);
      }
    } else if (_compare(*start_, *(end_ - 1))) {
      // middle <= start <= end
      pivot = utility::move(*start_);
      *start_ = utility::move(*middle);
    } else if (_compare(*middle, *(end_ - 1))) {
      // middle < end <= start
      pivot = utility::move(*(end_ - 1));
      *(end_ - 1) = utility::move(*start_);
      *start_ = utility::move(*middle);
    } else {
      pivot = utility::move(*middle);
      utility::swap(*start_, *(end_ - 1));
    }

    do {
      while (_compare(*item1, pivot)) {
        if (++item1 >= item2) {
          goto partitioned;
        }
      }

      while (_compare(pivot, *--item2)) {
        if (item1 >= item2) {
          goto partitioned;
        }
      }

      utility::swap(*item1, *item2);
    } while (++item1 < item2);

partitioned:
    *(end_ - 2) = utility::move(*item1);
    *item1 = utility::move(pivot);

    if (item1 - start_ < end_ - item1 + 1) {
      quick_sort(start_, item1, utility::forward<F>(_compare));
      start_ = item1 + 1;
    } else {
      quick_sort(item1 + 1, end_, utility::forward<F>(_compare));
      end_ = item1;
    }
  }

  insertion_sort(start_, end_, utility::forward<F>(_compare));
}

} // namespace rx::algorithm

#endif // RX_CORE_ALGORITHM_QUICK_SORT_H
