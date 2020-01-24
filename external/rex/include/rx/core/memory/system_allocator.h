#ifndef RX_CORE_MEMORY_SYSTEM_ALLOCATOR_H
#define RX_CORE_MEMORY_SYSTEM_ALLOCATOR_H
#include "rx/core/memory/heap_allocator.h"
#include "rx/core/memory/stats_allocator.h"

#include "rx/core/global.h"

namespace rx::memory {

// # System Allocator
//
// The generalized system allocator. Built off a heap allocator and a stats
// allocator to track global system allocations. When something isn't provided
// an allocator, this is the allocator used. More specifically, the global
// g_system_allocator is used.
struct system_allocator
  final : allocator
{
  system_allocator();

  virtual rx_byte* allocate(rx_size _size);
  virtual rx_byte* reallocate(rx_byte* _data, rx_size _size);
  virtual void deallocate(rx_byte* _data);

  stats_allocator::statistics stats();

private:
  heap_allocator m_heap_allocator;
  stats_allocator m_stats_allocator;
};

inline stats_allocator::statistics system_allocator::stats() {
  return m_stats_allocator.stats();
}

extern RX_GLOBAL<system_allocator> g_system_allocator;

} // namespace rx::memory

#endif // RX_CORE_MEMORY_SYSTEM_ALLOCATOR_H
