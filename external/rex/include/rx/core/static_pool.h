#ifndef RX_CORE_STATIC_POOL_H
#define RX_CORE_STATIC_POOL_H
#include "rx/core/bitset.h"
#include "rx/core/assert.h"

#include "rx/core/hints/unlikely.h"
#include "rx/core/hints/empty_bases.h"

namespace rx {

struct RX_HINT_EMPTY_BASES static_pool
  : concepts::no_copy
{
  static_pool(memory::allocator* _allocator, rx_size _object_size, rx_size _object_count);
  static_pool(rx_size _object_size, rx_size _object_count);
  static_pool(static_pool&& pool_);
  ~static_pool();

  static_pool& operator=(static_pool&& pool_);
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
  bool is_empty() const;

  rx_byte* data_of(rx_size _index) const;
  rx_size index_of(const rx_byte* _data) const;

  bool owns(const rx_byte* _data) const;

private:
  memory::allocator* m_allocator;
  rx_size m_object_size;
  rx_size m_capacity;
  rx_byte* m_data;
  bitset m_bitset;
};

inline static_pool::static_pool(rx_size _object_size, rx_size _object_count)
  : static_pool{&memory::g_system_allocator, _object_size, _object_count}
{
}

inline static_pool::~static_pool() {
  RX_ASSERT(m_bitset.count_set_bits() == 0, "leaked objects");
  m_allocator->deallocate(m_data);
}

inline rx_byte* static_pool::operator[](rx_size _index) const {
  return data_of(_index);
}

template<typename T, typename... Ts>
inline T* static_pool::create(Ts&&... _arguments) {
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
void static_pool::destroy(T* _data) {
  RX_ASSERT(sizeof(T) <= m_object_size, "object too large (%zu > %zu)",
    sizeof(T), m_object_size);

  utility::destruct<T>(_data);
  deallocate(index_of(reinterpret_cast<rx_byte*>(_data)));
}

inline memory::allocator* static_pool::allocator() const {
  return m_allocator;
}

inline rx_size static_pool::object_size() const {
  return m_object_size;
}

inline rx_size static_pool::capacity() const {
  return m_capacity;
}

inline rx_size static_pool::size() const {
  return m_bitset.count_set_bits();
}

inline bool static_pool::is_empty() const {
  return size() == 0;
}

inline rx_byte* static_pool::data_of(rx_size _index) const {
  RX_ASSERT(_index < m_capacity, "out of bounds");
  RX_ASSERT(m_bitset.test(_index), "unallocated (%zu)", _index);
  return m_data + m_object_size * _index;
}

inline rx_size static_pool::index_of(const rx_byte* _data) const {
  RX_ASSERT(owns(_data), "invalid pointer");
  return (_data - m_data) / m_object_size;
}

inline bool static_pool::owns(const rx_byte* _data) const {
  return _data >= m_data && _data <= m_data + m_object_size * (m_capacity - 1);
}

} // namespace rx

#endif // RX_CORE_POOL_H
