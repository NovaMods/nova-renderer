#include "rx/core/dynamic_pool.h"

namespace rx {

dynamic_pool::dynamic_pool(dynamic_pool&& pool_)
  : m_allocator{pool_.m_allocator}
  , m_pools{utility::move(pool_.m_pools)}
{
  pool_.m_allocator = nullptr;
}

dynamic_pool::~dynamic_pool() {
  m_pools.each_fwd([this](static_pool* _pool) {
    m_allocator->destroy<static_pool>(_pool);
  });
}

dynamic_pool& dynamic_pool::operator=(dynamic_pool&& pool_) {
  m_allocator = pool_.m_allocator;
  m_pools = utility::move(pool_.m_pools);
  pool_.m_allocator = nullptr;
  return *this;
}

rx_size dynamic_pool::pool_index_of(const rx_byte* _data) const {
  return m_pools.find_if([_data](const static_pool* _pool) {
    return _pool->owns(_data);
  });
}

rx_byte* dynamic_pool::data_of(rx_size _index) const {
  const rx_size pool_index = _index / m_pools.size();
  const rx_size object_index = _index % m_pools.size();
  return m_pools[pool_index]->data_of(object_index);
}

rx_size dynamic_pool::index_of(const rx_byte* _data) const {
  if (const rx_size index = pool_index_of(_data); index != -1_z) {
    return index * m_pools.size();
  }
  return -1_z;
}

void dynamic_pool::add_pool() {
  auto pool = m_allocator->create<static_pool>(m_allocator, m_object_size, m_objects_per_pool);
  m_pools.push_back(pool);
}

} // namespace rx
