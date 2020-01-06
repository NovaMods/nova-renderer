#ifndef RX_CORE_MEMORY_BUMP_POINT_ALLOCATOR_H
#define RX_CORE_MEMORY_BUMP_POINT_ALLOCATOR_H
#include "rx/core/memory/allocator.h"
#include "rx/core/concurrency/spin_lock.h"

namespace rx::memory {

// # Bump Point Allocator
//
// The idea behind a bump point allocator is to start with a pointer at the
// beginning of a fixed-sized block of memory and bump it forward by the
// size of an allocation. This is a very basic, but fast allocation strategy.
//
// Unlike other allocation algorithms, a bump point allocator has an extremely
// fast way to deallocate everything, just reset the bump pointer to the
// beginning of the fixed-size block of memory.
//
// The implementation here is a bit more intelligent though since it supports
// reallocation and deallocation provided the pointer passed to either is the
// same as the last pointer returned by a call to allocate.
//
// The purpose of this allocator is to provide a very quick, linear burn 
// scratch space to allocate shortly-lived objects and to reset.
struct bump_point_allocator
  final : allocator
{
  bump_point_allocator() = delete;

  // |_memory| must be aligned by allocator::k_alignment and |_size| must be
  // a multiple of allocator::k_alignment.
  bump_point_allocator(rx_byte* _memory, rx_size _size);

  virtual rx_byte* allocate(rx_size _size);
  virtual rx_byte* reallocate(rx_byte* _data, rx_size _size);
  virtual void deallocate(rx_byte* data);

  void reset();

  rx_size used() const;
  rx_size size() const;
  rx_size available() const;

private:
  rx_size m_size;
  rx_byte* m_data;

  concurrency::spin_lock m_lock;
  rx_byte* m_this_point; // protected by |m_lock|
  rx_byte* m_last_point; // protected by |m_lock|
};

inline rx_size bump_point_allocator::used() const {
  return m_this_point - m_data;
}

inline rx_size bump_point_allocator::size() const {
  return m_size;
}

inline rx_size bump_point_allocator::available() const {
  return size() - used();
}

} // namespace rx::memory

#endif // RX_CORE_MEMORY_BUMP_POINT_ALLOCATOR_H
