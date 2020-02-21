#ifndef RX_CORE_SET_H
#define RX_CORE_SET_H
#include "rx/core/utility/move.h"
#include "rx/core/utility/swap.h"

#include "rx/core/traits/return_type.h"
#include "rx/core/traits/is_same.h"

#include "rx/core/hints/unreachable.h"

#include "rx/core/memory/system_allocator.h" // allocator, g_system_allocator

#include "rx/core/hash.h"

namespace rx {

// 32-bit: 28 bytes
// 64-bit: 56 bytes
template<typename K>
struct set {
  template<typename Kt, rx_size E>
  using initializers = array<Kt[E]>;

  static constexpr rx_size k_initial_size{256};
  static constexpr rx_size k_load_factor{90};

  set();
  set(memory::allocator* _allocator);
  set(set&& set_);
  set(const set& _set);

  template<typename Kt, rx_size E>
  set(memory::allocator* _allocator, initializers<Kt, E>&& initializers_);

  template<typename Kt, rx_size E>
  set(initializers<Kt, E>&& initializers_);

  ~set();

  set& operator=(set&& set_);
  set& operator=(const set& _set);

  void insert(K&& _key);
  void insert(const K& _key);

  bool find(const K& _key) const;

  bool erase(const K& _key);
  rx_size size() const;
  bool is_empty() const;

  void clear();

  template<typename F>
  bool each(F&& _function);
  template<typename F>
  bool each(F&& _function) const;

  memory::allocator* allocator() const;

private:
  void clear_and_deallocate();
  constexpr void initialize(memory::allocator* _allocator, rx_size _capacity);

  static rx_size hash_key(const K& _key);
  static bool is_deleted(rx_size _hash);

  rx_size desired_position(rx_size _hash) const;
  rx_size probe_distance(rx_size _hash, rx_size _slot_index) const;

  rx_size& element_hash(rx_size index);
  rx_size element_hash(rx_size index) const;

  void allocate();
  void grow();

  // move and non-move construction functions
  void construct(rx_size _index, rx_size _hash, K&& key_);

  void inserter(rx_size _hash, K&& key_);
  void inserter(rx_size _hash, const K& _key);

  bool lookup_index(const K& _key, rx_size& _index) const;

  memory::allocator* m_allocator;

  K* m_keys;
  rx_size* m_hashes;

  rx_size m_size;
  rx_size m_capacity;
  rx_size m_resize_threshold;
  rx_size m_mask;
};

template<typename K>
inline set<K>::set()
  : set{&memory::g_system_allocator}
{
}

template<typename K>
inline set<K>::set(memory::allocator* _allocator)
{
  initialize(_allocator, k_initial_size);
  allocate();
}

template<typename K>
inline set<K>::set(set&& set_)
  : m_allocator{set_.m_allocator}
  , m_keys{set_.m_keys}
  , m_hashes{set_.m_hashes}
  , m_size{set_.m_size}
  , m_capacity{set_.m_capacity}
  , m_resize_threshold{set_.m_resize_threshold}
  , m_mask{set_.m_mask}
{
  set_.initialize(&memory::g_system_allocator, 0);
}

template<typename K>
inline set<K>::set(const set& _set)
  : set{_set.m_allocator}
{
  for (rx_size i{0}; i < _set.m_capacity; i++) {
    const auto hash = _set.element_hash(i);
    if (hash != 0 && !_set.is_deleted(hash)) {
      insert(_set.m_keys[i]);
    }
  }
}

template<typename K>
template<typename Kt, rx_size E>
inline set<K>::set(memory::allocator* _allocator, initializers<Kt, E>&& initializers_)
  : set{_allocator}
{
  for (rx_size i = 0; i < E; i++) {
    insert(utility::move(initializers_[i]));
  }
}

template<typename K>
template<typename Kt, rx_size E>
inline set<K>::set(initializers<Kt, E>&& initializers_)
  : set{&memory::g_system_allocator, utility::move(initializers_)}
{
}

template<typename K>
inline set<K>::~set() {
  clear_and_deallocate();
}

template<typename K>
inline void set<K>::clear() {
  if (m_size == 0) {
    return;
  }

  for (rx_size i{0}; i < m_capacity; i++) {
    const rx_size hash = element_hash(i);
    if (hash != 0 && !is_deleted(hash)) {
      if constexpr (!traits::is_trivially_destructible<K>) {
        utility::destruct<K>(m_keys + i);
      }
      element_hash(i) = 0;
    }
  }

  m_size = 0;
}

template<typename K>
inline void set<K>::clear_and_deallocate() {
  clear();

  m_allocator->deallocate(reinterpret_cast<rx_byte*>(m_keys));
  m_allocator->deallocate(reinterpret_cast<rx_byte*>(m_hashes));
}

template<typename K>
inline set<K>& set<K>::operator=(set<K>&& set_) {
  RX_ASSERT(&set_ != this, "self assignment");

  clear_and_deallocate();

  m_allocator = set_.m_allocator;
  m_keys = set_.m_keys;
  m_hashes = set_.m_hashes;
  m_size = set_.m_size;
  m_capacity = set_.m_capacity;
  m_resize_threshold = set_.m_resize_threshold;
  m_mask = set_.m_mask;

  set_.initialize(&memory::g_system_allocator, 0);

  return *this;
}

template<typename K>
inline set<K>& set<K>::operator=(const set<K>& _set) {
  RX_ASSERT(&_set != this, "self assignment");

  clear_and_deallocate();
  initialize(_set.m_allocator, _set.m_capacity);
  allocate();

  for (rx_size i{0}; i < _set.m_capacity; i++) {
    const auto hash = _set.element_hash(i);
    if (hash != 0 && !_set.is_deleted(hash)) {
      insert(_set.m_keys[i]);
    }
  }

  return *this;
}

template<typename K>
inline constexpr void set<K>::initialize(memory::allocator* _allocator, rx_size _capacity) {
  m_allocator = _allocator;
  m_keys = nullptr;
  m_hashes = nullptr;
  m_size = 0;
  m_capacity = _capacity;
  m_resize_threshold = 0;
  m_mask = 0;
}

template<typename K>
inline void set<K>::insert(K&& key_) {
  if (++m_size >= m_resize_threshold) {
    grow();
  }
  inserter(hash_key(key_), utility::forward<K>(key_));
}

template<typename K>
inline void set<K>::insert(const K& _key) {
  if (++m_size >= m_resize_threshold) {
    grow();
  }
  inserter(hash_key(_key), _key);
}

template<typename K>
bool set<K>::find(const K& _key) const {
  if (rx_size index; lookup_index(_key, index)) {
    return true;
  }
  return false;
}

template<typename K>
inline bool set<K>::erase(const K& _key) {
  if (rx_size index; lookup_index(_key, index)) {
    if constexpr (!traits::is_trivially_destructible<K>) {
      utility::destruct<K>(m_keys + index);
    }

    if constexpr (sizeof index == 8) {
      element_hash(index) |= 0x8000000000000000;
    } else {
      element_hash(index) |= 0x80000000;
    }

    m_size--;
    return true;
  }
  return false;
}

template<typename K>
inline rx_size set<K>::size() const {
  return m_size;
}

template<typename K>
inline bool set<K>::is_empty() const {
  return m_size == 0;
}

template<typename K>
inline rx_size set<K>::hash_key(const K& _key) {
  auto hash_value{hash<K>{}(_key)};

  // MSB is used to indicate deleted elements
  if constexpr(sizeof hash_value == 8) {
    hash_value &= 0x7FFFFFFFFFFFFFFF_z;
  } else {
    hash_value &= 0x7FFFFFFF_z;
  }

  // don't ever hash to zero since zero is used to indicate unused slots
  hash_value |= hash_value == 0;

  return hash_value;
}

template<typename K>
inline bool set<K>::is_deleted(rx_size _hash) {
  // MSB indicates tombstones
  return (_hash >> ((sizeof _hash * 8) - 1)) != 0;
}

template<typename K>
inline rx_size set<K>::desired_position(rx_size _hash) const {
  return _hash & m_mask;
}

template<typename K>
inline rx_size set<K>::probe_distance(rx_size _hash, rx_size _slot_index) const {
  return (_slot_index + m_capacity - desired_position(_hash)) & m_mask;
}

template<typename K>
inline rx_size& set<K>::element_hash(rx_size _index) {
  return m_hashes[_index];
}

template<typename K>
inline rx_size set<K>::element_hash(rx_size _index) const {
  return m_hashes[_index];
}

template<typename K>
inline void set<K>::allocate() {
  m_keys = reinterpret_cast<K*>(m_allocator->allocate(sizeof(K) * m_capacity));
  m_hashes = reinterpret_cast<rx_size*>(m_allocator->allocate(sizeof(rx_size) * m_capacity));

  for (rx_size i{0}; i < m_capacity; i++) {
    element_hash(i) = 0;
  }

  m_resize_threshold = (m_capacity * k_load_factor) / 100;
  m_mask = m_capacity - 1;
}

template<typename K>
inline void set<K>::grow() {
  const auto old_capacity{m_capacity};

  auto keys_data{m_keys};
  auto hashes_data{m_hashes};

  m_capacity *= 2;
  allocate();

  for (rx_size i{0}; i < old_capacity; i++) {
    const auto hash{hashes_data[i]};
    if (hash != 0 && !is_deleted(hash)) {
      inserter(hash, utility::move(keys_data[i]));
      if constexpr (!traits::is_trivially_destructible<K>) {
        utility::destruct<K>(keys_data + i);
      }
    }
  }

  m_allocator->deallocate(reinterpret_cast<rx_byte*>(keys_data));
  m_allocator->deallocate(reinterpret_cast<rx_byte*>(hashes_data));
}

template<typename K>
inline void set<K>::construct(rx_size _index, rx_size _hash, K&& key_) {
  utility::construct<K>(m_keys + _index, utility::forward<K>(key_));
  element_hash(_index) = _hash;
}

template<typename K>
inline void set<K>::inserter(rx_size _hash, K&& key_) {
  rx_size position{desired_position(_hash)};
  rx_size distance{0};
  for (;;) {
    if (element_hash(position) == 0) {
      construct(position, _hash, utility::forward<K>(key_));
      return;
    }

    const rx_size existing_element_probe_distance{probe_distance(element_hash(position), position)};
    if (existing_element_probe_distance < distance) {
      if (is_deleted(element_hash(position))) {
        construct(position, _hash, utility::forward<K>(key_));
        return;
      }

      utility::swap(_hash, element_hash(position));
      utility::swap(key_, m_keys[position]);

      distance = existing_element_probe_distance;
    }

    position = (position + 1) & m_mask;
    distance++;
  }

  RX_HINT_UNREACHABLE();
}

template<typename K>
inline void set<K>::inserter(rx_size _hash, const K& _key) {
  K key{_key};
  inserter(_hash, utility::move(key));
}

template<typename K>
inline bool set<K>::lookup_index(const K& _key, rx_size& _index) const {
  const rx_size hash{hash_key(_key)};
  rx_size position{desired_position(hash)};
  rx_size distance{0};
  for (;;) {
    const rx_size hash_element{element_hash(position)};
    if (hash_element == 0) {
      return false;
    } else if (distance > probe_distance(hash_element, position)) {
      return false;
    } else if (hash_element == hash && m_keys[position] == _key) {
      _index = position;
      return true;
    }
    position = (position + 1) & m_mask;
    distance++;
  }

  RX_HINT_UNREACHABLE();
}

template<typename K>
template<typename F>
inline bool set<K>::each(F&& _function) {
  for (rx_size i{0}; i < m_capacity; i++) {
    const auto hash{m_hashes[i]};
    if (hash != 0 && !is_deleted(hash)) {
      if constexpr (traits::is_same<traits::return_type<F>, bool>) {
        if (!_function(m_keys[i])) {
          return false;
        }
      } else {
        _function(m_keys[i]);
      }
    }
  }
  return true;
}

template<typename K>
template<typename F>
inline bool set<K>::each(F&& _function) const {
  for (rx_size i{0}; i < m_capacity; i++) {
    const auto hash{m_hashes[i]};
    if (hash != 0 && !is_deleted(hash)) {
      if constexpr (traits::is_same<traits::return_type<F>, bool>) {
        if (!_function(m_keys[i])) {
          return false;
        }
      } else {
        _function(m_keys[i]);
      }
    }
  }
  return true;
}

template<typename K>
inline memory::allocator* set<K>::allocator() const {
  return m_allocator;
}

} // namespace rx

#endif // RX_CORE_SET
