#ifndef RX_CORE_BITSET_H
#define RX_CORE_BITSET_H
#include "rx/core/traits/is_same.h"
#include "rx/core/traits/return_type.h"

#include "rx/core/memory/system_allocator.h"

#include "rx/core/assert.h"

namespace rx {

// 32-bit: 12 bytes
// 64-bit: 24 bytes
struct bitset {
  using bit_type = rx_u64;

  static constexpr const bit_type k_bit_one{1};
  static constexpr const rx_size k_word_bits{8 * sizeof(bit_type)};

  bitset(memory::allocator* _allocator, rx_size _size);
  bitset(rx_size _size);
  bitset(bitset&& bitset_);
  bitset(const bitset& _bitset);
  ~bitset();

  bitset& operator=(bitset&& bitset_);
  bitset& operator=(const bitset& _bitset);

  // set |_bit|
  void set(rx_size _bit);

  // clear |_bit|
  void clear(rx_size _bit);

  // clear all bits
  void clear_all();

  // test if bit |_bit| is set
  bool test(rx_size _bit) const;

  // the amount of bits
  rx_size size() const;

  // count the # of set bits
  rx_size count_set_bits() const;

  // count the # of unset bits
  rx_size count_unset_bits() const;

  // find the index of the first set bit
  rx_size find_first_set() const;

  // find the index of the first unset bit
  rx_size find_first_unset() const;

  // iterate bitset invoking |_function| with index of each set bit
  template<typename F>
  void each_set(F&& _function) const;

  // iterate bitset invoking |_function| with index of each unset bit
  template<typename F>
  void each_unset(F&& _function) const;

  memory::allocator* allocator() const;

private:
  static rx_size bytes_for_size(rx_size _size);

  static rx_size index(rx_size bit);
  static rx_size offset(rx_size bit);

  memory::allocator* m_allocator;

  rx_size m_size;
  bit_type* m_data;
};

inline bitset::bitset(rx_size _size)
  : bitset{&memory::g_system_allocator, _size}
{
}

inline bitset::bitset(bitset&& bitset_)
  : m_allocator{bitset_.m_allocator}
  , m_data{bitset_.m_data}
{
  bitset_.m_size = 0;
  bitset_.m_data = nullptr;
}

inline bitset::~bitset() {
  m_allocator->deallocate(reinterpret_cast<rx_byte*>(m_data));
}

inline void bitset::set(rx_size _bit) {
  RX_ASSERT(_bit < m_size, "out of bounds");
  m_data[index(_bit)] |= k_bit_one << offset(_bit);
}

inline void bitset::clear(rx_size _bit) {
  RX_ASSERT(_bit < m_size, "out of bounds");
  m_data[index(_bit)] &= ~(k_bit_one << offset(_bit));
}

inline bool bitset::test(rx_size _bit) const {
  RX_ASSERT(_bit < m_size, "out of bounds");
  return !!(m_data[index(_bit)] & (k_bit_one << offset(_bit)));
}

inline rx_size bitset::size() const {
  return m_size;
}

inline rx_size bitset::bytes_for_size(rx_size _size) {
  return sizeof(bit_type) * (_size / k_word_bits + 1);
}

inline rx_size bitset::index(rx_size _bit) {
  return _bit / k_word_bits;
}

inline rx_size bitset::offset(rx_size _bit) {
  return _bit % k_word_bits;
}

template<typename F>
inline void bitset::each_set(F&& _function) const {
  for (rx_size i{0}; i < m_size; i++) {
    if (test(i)) {
      if constexpr (traits::is_same<bool, traits::return_type<F>>) {
        if (!_function(i)) {
          return;
        }
      } else {
        _function(i);
      }
    }
  }
}

template<typename F>
inline void bitset::each_unset(F&& _function) const {
  for (rx_size i{0}; i < m_size; i++) {
    if (!test(i)) {
      if constexpr (traits::is_same<bool, traits::return_type<F>>) {
        if (!_function(i)) {
          return;
        }
      } else {
        _function(i);
      }
    }
  }
}

inline memory::allocator* bitset::allocator() const {
  return m_allocator;
}

} // namespace rx

#endif // RX_CORE_BITSET_H
