#include "rx/core/memory/bump_point_allocator.h"
#include "rx/core/concurrency/scope_lock.h"

#include "rx/core/hints/likely.h"
#include "rx/core/hints/unlikely.h"

#include "rx/core/assert.h"

namespace rx::memory {

bump_point_allocator::bump_point_allocator(rx_byte* _data, rx_size _size)
  : m_size{_size}
  , m_data{_data}
  , m_this_point{m_data}
  , m_last_point{m_data}
{
  // Ensure the memory given is suitably aligned and size is suitably rounded.
  RX_ASSERT(reinterpret_cast<rx_uintptr>(m_data) % k_alignment == 0,
    "_data not aligned on k_alignment boundary");
  RX_ASSERT(m_size % k_alignment == 0,
    "_size not a multiple of k_alignment");
}

rx_byte* bump_point_allocator::allocate(rx_size _size) {
  concurrency::scope_lock locked{m_lock};

  // Round |_size| to a multiple of k_alignment to keep all pointers
  // aligned by k_alignment.
  _size = allocator::round_to_alignment(_size);

  // Check for available space for the allocation.
  if (RX_HINT_UNLIKELY(m_this_point + _size >= m_data + m_size)) {
    return nullptr;
  }

  // Backup the last point to make deallocation and reallocation possible.
  m_last_point = m_this_point;

  // Bump the point along by the rounded allocation size.
  m_this_point += _size;

  return m_last_point;
}

rx_byte* bump_point_allocator::reallocate(rx_byte* _data, rx_size _size) {
  if (RX_HINT_LIKELY(_data)) {
    concurrency::scope_lock locked{m_lock};

    // Round |_size| to a multiple of k_alignment to keep all pointers
    // aligned by k_alignment.
    _size = allocator::round_to_alignment(_size);

    // Can only reallocate in-place provided |_data| is the address of |m_last_point|,
    // i.e it's the most recently allocated.
    if (RX_HINT_LIKELY(_data == m_last_point)) {
      // Check for available space for the allocation.
      if (RX_HINT_UNLIKELY(m_last_point + _size >= m_data + m_size)) {
        return nullptr;
      }

      // Bump the pointer along by the last point and new size.
      m_this_point = m_last_point + _size;

      return _data;
    }

    return nullptr;
  }

  return allocate(_size);
}

void bump_point_allocator::deallocate(rx_byte* _data) {
  concurrency::scope_lock locked{m_lock};

  // Can only deallocate provided |_data| is the address of |m_last_point|,
  // i.e it's the most recently allocated or reallocated.
  if (RX_HINT_LIKELY(_data == m_last_point)) {
    m_this_point = m_last_point;
  }
}

void bump_point_allocator::reset() {
  concurrency::scope_lock locked{m_lock};

  m_this_point = m_data;
  m_last_point = m_data;
}

} // namespace rx::memory
