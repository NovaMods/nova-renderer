#ifndef RX_CORE_POOL_H
#define RX_CORE_POOL_H
#include "rx/core/bitset.h"
#include "rx/core/assert.h"

#include "rx/core/hints/unlikely.h"

namespace rx {

struct pool
  : concepts::no_copy
{
  pool(memory::allocator* _allocator, rx_size _object_size, rx_size _object_count);
  pool(rx_size _object_size, rx_size _object_count);
  pool(pool&& pool_);
  ~pool();

  pool& operator=(pool&& pool_);
  rx_byte* operator[](rx_size _index) const;

  rx_size allocate();
  void deallocate(rx_size _index);

  template<typename T, typename... Ts>
  T* create(Ts&&... _arguments);

  template<typename T>
  void destroy(T* _data);

  memory::allocator* allocator() const;

  rx_size object_size() const;
  rx_size capacity() const;
  rx_size size() const;

  rx_byte* data_of(rx_size _index) const;
  rx_size index_of(const rx_byte* _data) const;

private:
  memory::allocator* m_allocator;
  rx_size m_object_size;
  rx_size m_capacity;
  rx_byte* m_data;
  bitset m_bitset;
};

inline pool::pool(rx_size _object_size, rx_size _object_count)
  : pool{&memory::g_system_allocator, _object_size, _object_count}
{
}

inline pool::~pool() {
  RX_ASSERT(m_bitset.count_set_bits() == 0, "leaked objects");
  m_allocator->deallocate(m_data);
}

inline rx_byte* pool::operator[](rx_size _index) const {
  return data_of(_index);
}

template<typename T, typename... Ts>
inline T* pool::create(Ts&&... _arguments) {
  RX_ASSERT(sizeof(T) <= m_object_size, "object too large (%zu > %zu)",
    sizeof(T), m_object_size);

  const rx_size index{allocate()};
  if (RX_HINT_UNLIKELY(index == -1_z)) {
    return nullptr;
  }

  return utility::construct<T>(data_of(index),
    utility::forward<Ts>(_arguments)...);
}

template<typename T>
void pool::destroy(T* _data) {
  RX_ASSERT(sizeof(T) <= m_object_size, "object too large (%zu > %zu)",
    sizeof(T), m_object_size);

  utility::destruct<T>(_data);
  deallocate(index_of(reinterpret_cast<rx_byte*>(_data)));
}

inline memory::allocator* pool::allocator() const {
  return m_allocator;
}

inline rx_size pool::object_size() const {
  return m_object_size;
}

inline rx_size pool::capacity() const {
  return m_capacity;
}

inline rx_size pool::size() const {
  return m_bitset.count_set_bits();
}

inline rx_byte* pool::data_of(rx_size _index) const {
  RX_ASSERT(_index < m_capacity, "out of bounds");
  RX_ASSERT(m_bitset.test(_index), "unallocated (%zu)", _index);
  return m_data + m_object_size * _index;
}

inline rx_size pool::index_of(const rx_byte* _data) const {
  RX_ASSERT(_data >= m_data && _data <= m_data + m_object_size * (m_capacity - 1), "invalid pointer");
  return (_data - m_data) / m_object_size;
}

} // namespace rx

#endif // RX_CORE_POOL_H
