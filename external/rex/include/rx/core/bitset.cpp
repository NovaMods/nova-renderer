#include <string.h> // memset

#include "rx/core/bitset.h" // bitset
#include "rx/core/assert.h" // RX_ASSERT

#include "rx/core/memory/system_allocator.h" // g_system_allocator

namespace rx {

bitset::bitset(memory::allocator* _allocator, rx_size _size)
  : m_allocator{_allocator}
  , m_size{_size}
  , m_data{reinterpret_cast<bit_type*>(m_allocator->allocate(bytes_for_size(m_size)))}
{
  RX_ASSERT(m_data, "out of memory");

  clear_all();
}

bitset::bitset(const bitset& _bitset)
  : m_allocator{_bitset.m_allocator}
  , m_size{_bitset.m_size}
  , m_data{reinterpret_cast<bit_type*>(m_allocator->allocate(bytes_for_size(m_size)))}
{
  RX_ASSERT(m_data, "out of memory");

  memcpy(m_data, _bitset.m_data, bytes_for_size(m_size));
}

bitset& bitset::operator=(bitset&& bitset_) {
  RX_ASSERT(&bitset_ != this, "self assignment");

  m_allocator = bitset_.m_allocator;
  m_size = bitset_.m_size;
  m_data = bitset_.m_data;

  bitset_.m_size = 0;
  bitset_.m_data = nullptr;

  return *this;
}

bitset& bitset::operator=(const bitset& _bitset) {
  RX_ASSERT(&_bitset != this, "self assignment");

  m_allocator->deallocate(reinterpret_cast<rx_byte*>(m_data));

  m_allocator = _bitset.m_allocator;
  m_size = _bitset.m_size;
  m_data = reinterpret_cast<rx_u64*>(m_allocator->allocate(bytes_for_size(m_size)));
  RX_ASSERT(m_data, "out of memory");

  memcpy(m_data, _bitset.m_data, bytes_for_size(m_size));

  return *this;
}

void bitset::clear_all() {
  memset(m_data, 0, bytes_for_size(m_size));
}

rx_size bitset::count_set_bits() const {
  rx_size count{0};

  for (rx_size i{0}; i < m_size; i++) {
    if (test(i)) {
      count++;
    }
  }

  return count;
}

rx_size bitset::count_unset_bits() const {
  rx_size count{0};

  for (rx_size i{0}; i < m_size; i++) {
    if (!test(i)) {
      count++;
    }
  }

  return count;
}

rx_size bitset::find_first_unset() const {
  for (rx_size i{0}; i < m_size; i++) {
    if (!test(i)) {
      return i;
    }
  }
  return -1_z;
}

rx_size bitset::find_first_set() const {
  for (rx_size i{0}; i < m_size; i++) {
    if (test(i)) {
      return i;
    }
  }
  return -1_z;
}

} // namespace rx
