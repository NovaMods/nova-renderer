#ifndef RX_CORE_MEMORY_STATS_ALLOCATOR_H
#define RX_CORE_MEMORY_STATS_ALLOCATOR_H
#include "rx/core/memory/allocator.h"
#include "rx/core/concurrency/spin_lock.h"

namespace rx::memory {

// # Statistics Allocator
//
// The idea behind a statistics allocator is to wrap an existing allocator
// implementation and provide statistics for it. This is done by extending the
// size of the allocations to make room for additional meta-data.
//
// The purpose of this allocator is to provide a means to debug and track
// information about any allocator.
struct stats_allocator
  final : allocator
{
  stats_allocator() = delete;
  stats_allocator(allocator* _allocator);

  virtual rx_byte* allocate(rx_size _size);
  virtual rx_byte* reallocate(rx_byte* _data, rx_size _size);
  virtual void deallocate(rx_byte* _data);

  struct statistics {
    rx_size allocations;           // Number of calls to allocate
    rx_size request_reallocations; // Number of calls to reallocate in total
    rx_size actual_reallocations;  // Number of calls to reallocate that actually in-place reallocated
    rx_size deallocations;         // Number of calls to deallocate

    // Measures peak and in-use requested bytes.
    // Requested bytes are the sizes passed to allocate and reallocate.
    rx_u64 peak_request_bytes;
    rx_u64 used_request_bytes;

    // Measures peak and in-use actual bytes.
    // Actual bytes are the sizes once rounded and adjusted to make room for metadata.
    rx_u64 peak_actual_bytes;
    rx_u64 used_actual_bytes;
  };

  statistics stats();

private:
  allocator* m_allocator;
  concurrency::spin_lock m_lock;
  statistics m_statistics; // protected by |m_lock|
};

} // namespace rx::memory

#endif // RX_CORE_MEMORY_STATS_ALLOCATOR_H

