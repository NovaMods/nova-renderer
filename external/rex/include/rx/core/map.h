#ifndef RX_CORE_MAP_H
#define RX_CORE_MAP_H
#include "rx/core/utility/swap.h"
#include "rx/core/utility/pair.h"

#include "rx/core/traits/return_type.h"
#include "rx/core/traits/is_trivially_destructible.h"
#include "rx/core/traits/is_same.h"

#include "rx/core/hints/unreachable.h"

#include "rx/core/memory/system_allocator.h" // allocator, g_system_allocator

#include "rx/core/hash.h"
#include "rx/core/array.h"

namespace rx {

// 32-bit: 28 bytes
// 64-bit: 56 bytes
template<typename K, typename V>
struct map {
  template<typename Kt, typename Vt, rx_size E>
  using initializers = array<pair<Kt, Vt>[E]>;

  static constexpr rx_size k_initial_size{256};
  static constexpr rx_size k_load_factor{90};

  map();
  map(memory::allocator* _allocator);
  map(map&& map_);
  map(const map& _map);

  template<typename Kt, typename Vt, rx_size E>
  map(memory::allocator* _allocator, initializers<Kt, Vt, E>&& initializers_);

  template<typename Kt, typename Vt, rx_size E>
  map(initializers<Kt, Vt, E>&& initializers_);

  ~map();

  map& operator=(map&& map_);
  map& operator=(const map& _map);

  V* insert(const K& _key, V&& value_);
  V* insert(const K& _key, const V& _value);

  V* find(const K& _key);
  const V* find(const K& _key) const;

  bool erase(const K& _key);
  rx_size size() const;
  bool is_empty() const;

  void clear();

  template<typename F>
  bool each_key(F&& _function);
  template<typename F>
  bool each_key(F&& _function) const;

  template<typename F>
  bool each_value(F&& _function);
  template<typename F>
  bool each_value(F&& _function) const;

  template<typename F>
  bool each_pair(F&& _function);
  template<typename F>
  bool each_pair(F&& _function) const;

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
  V* construct(rx_size _index, rx_size _hash, K&& key_, V&& value_);

  V* inserter(rx_size _hash, K&& key_, V&& value_);
  V* inserter(rx_size _hash, const K& _key, const V& _value);
  V* inserter(rx_size _hash, const K& _key, V&& value_);

  bool lookup_index(const K& _key, rx_size& _index) const;

  memory::allocator* m_allocator;

  K* m_keys;
  V* m_values;
  rx_size* m_hashes;

  rx_size m_size;
  rx_size m_capacity;
  rx_size m_resize_threshold;
  rx_size m_mask;
};

template<typename K, typename V>
inline map<K, V>::map()
  : map{&memory::g_system_allocator}
{
}

template<typename K, typename V>
inline map<K, V>::map(memory::allocator* _allocator)
{
  initialize(_allocator, k_initial_size);
  allocate();
}

template<typename K, typename V>
inline map<K, V>::map(map&& map_)
  : m_allocator{map_.m_allocator}
  , m_keys{map_.m_keys}
  , m_values{map_.m_values}
  , m_hashes{map_.m_hashes}
  , m_size{map_.m_size}
  , m_capacity{map_.m_capacity}
  , m_resize_threshold{map_.m_resize_threshold}
  , m_mask{map_.m_mask}
{
  map_.initialize(&memory::g_system_allocator, 0);
}

template<typename K, typename V>
inline map<K, V>::map(const map& _map)
  : map{_map.m_allocator}
{
  for (rx_size i{0}; i < _map.m_capacity; i++) {
    const auto hash = _map.element_hash(i);
    if (hash != 0 && !_map.is_deleted(hash)) {
      insert(_map.m_keys[i], _map.m_values[i]);
    }
  }
}

template<typename K, typename V>
template<typename Kt, typename Vt, rx_size E>
inline map<K, V>::map(memory::allocator* _allocator, initializers<Kt, Vt, E>&& initializers_)
  : map{_allocator}
{
  for (rx_size i = 0; i < E; i++) {
    auto& item = initializers_[i];
    insert(utility::move(item.first), utility::move(item.second));
  }
}

template<typename K, typename V>
template<typename Kt, typename Vt, rx_size E>
inline map<K, V>::map(initializers<Kt, Vt, E>&& initializers_)
  : map{&memory::g_system_allocator, utility::move(initializers_)}
{
}

template<typename K, typename V>
inline map<K, V>::~map() {
  clear_and_deallocate();
}

template<typename K, typename V>
inline void map<K, V>::clear() {
  if (m_size == 0) {
    return;
  }

  for (rx_size i{0}; i < m_capacity; i++) {
    const auto hash = element_hash(i);
    if (hash != 0 && !is_deleted(hash)) {
      if constexpr (!traits::is_trivially_destructible<K>) {
        utility::destruct<K>(m_keys + i);
      }
      if constexpr (!traits::is_trivially_destructible<V>) {
        utility::destruct<V>(m_values + i);
      }
      element_hash(i) = 0;
    }
  }

  m_size = 0;
}

template<typename K, typename V>
inline void map<K, V>::clear_and_deallocate() {
  clear();

  m_allocator->deallocate(reinterpret_cast<rx_byte*>(m_keys));
  m_allocator->deallocate(reinterpret_cast<rx_byte*>(m_values));
  m_allocator->deallocate(reinterpret_cast<rx_byte*>(m_hashes));
}

template<typename K, typename V>
inline map<K, V>& map<K, V>::operator=(map<K, V>&& map_) {
  RX_ASSERT(&map_ != this, "self assignment");

  clear_and_deallocate();

  m_allocator = map_.m_allocator;
  m_keys = map_.m_keys;
  m_values = map_.m_values;
  m_hashes = map_.m_hashes;
  m_size = map_.m_size;
  m_capacity = map_.m_capacity;
  m_resize_threshold = map_.m_resize_threshold;
  m_mask = map_.m_mask;

  map_.initialize(&memory::g_system_allocator, 0);

  return *this;
}

template<typename K, typename V>
inline map<K, V>& map<K, V>::operator=(const map<K, V>& _map) {
  RX_ASSERT(&_map != this, "self assignment");

  clear_and_deallocate();
  initialize(_map.m_allocator, _map.m_capacity);
  allocate();

  for (rx_size i{0}; i < _map.m_capacity; i++) {
    const auto hash = _map.element_hash(i);
    if (hash != 0 && !_map.is_deleted(hash)) {
      insert(_map.m_keys[i], _map.m_values[i]);
    }
  }

  return *this;
}

template<typename K, typename V>
inline constexpr void map<K, V>::initialize(memory::allocator* _allocator, rx_size _capacity) {
  m_allocator = _allocator;
  m_keys = nullptr;
  m_values = nullptr;
  m_hashes = nullptr;
  m_size = 0;
  m_capacity = _capacity;
  m_resize_threshold = 0;
  m_mask = 0;
}

template<typename K, typename V>
inline V* map<K, V>::insert(const K& _key, V&& value_) {
  if (++m_size >= m_resize_threshold) {
    grow();
  }
  return inserter(hash_key(_key), _key, utility::forward<V>(value_));
}

template<typename K, typename V>
inline V* map<K, V>::insert(const K& _key, const V& _value) {
  if (++m_size >= m_resize_threshold) {
    grow();
  }
  return inserter(hash_key(_key), _key, _value);
}

template<typename K, typename V>
V* map<K, V>::find(const K& _key) {
  if (rx_size index; lookup_index(_key, index)) {
    return m_values + index;
  }
  return nullptr;
}

template<typename K, typename V>
const V* map<K, V>::find(const K& _key) const {
  if (rx_size index; lookup_index(_key, index)) {
    return m_values + index;
  }
  return nullptr;
}

template<typename K, typename V>
inline bool map<K, V>::erase(const K& _key) {
  if (rx_size index; lookup_index(_key, index)) {
    if constexpr (!traits::is_trivially_destructible<K>) {
      utility::destruct<K>(m_keys + index);
    }
    if constexpr (!traits::is_trivially_destructible<V>) {
      utility::destruct<V>(m_values + index);
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

template<typename K, typename V>
inline rx_size map<K, V>::size() const {
  return m_size;
}

template<typename K, typename V>
inline bool map<K, V>::is_empty() const {
  return m_size == 0;
}

template<typename K, typename V>
inline rx_size map<K, V>::hash_key(const K& _key) {
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

template<typename K, typename V>
inline bool map<K, V>::is_deleted(rx_size _hash) {
  // MSB indicates tombstones
  return (_hash >> ((sizeof _hash * 8) - 1)) != 0;
}

template<typename K, typename V>
inline rx_size map<K, V>::desired_position(rx_size _hash) const {
  return _hash & m_mask;
}

template<typename K, typename V>
inline rx_size map<K, V>::probe_distance(rx_size _hash, rx_size _slot_index) const {
  return (_slot_index + m_capacity - desired_position(_hash)) & m_mask;
}

template<typename K, typename V>
inline rx_size& map<K, V>::element_hash(rx_size _index) {
  return m_hashes[_index];
}

template<typename K, typename V>
inline rx_size map<K, V>::element_hash(rx_size _index) const {
  return m_hashes[_index];
}

template<typename K, typename V>
inline void map<K, V>::allocate() {
  m_keys = reinterpret_cast<K*>(m_allocator->allocate(sizeof(K) * m_capacity));
  m_values = reinterpret_cast<V*>(m_allocator->allocate(sizeof(V) * m_capacity));
  m_hashes = reinterpret_cast<rx_size*>(m_allocator->allocate(sizeof(rx_size) * m_capacity));

  for (rx_size i{0}; i < m_capacity; i++) {
    element_hash(i) = 0;
  }

  m_resize_threshold = (m_capacity * k_load_factor) / 100;
  m_mask = m_capacity - 1;
}

template<typename K, typename V>
inline void map<K, V>::grow() {
  const auto old_capacity{m_capacity};

  auto keys_data{m_keys};
  auto values_data{m_values};
  auto hashes_data{m_hashes};

  m_capacity *= 2;
  allocate();

  for (rx_size i{0}; i < old_capacity; i++) {
    const auto hash{hashes_data[i]};
    if (hash != 0 && !is_deleted(hash)) {
      inserter(hash, utility::move(keys_data[i]), utility::move(values_data[i]));
      if constexpr (!traits::is_trivially_destructible<K>) {
        utility::destruct<K>(keys_data + i);
      }
      if constexpr (!traits::is_trivially_destructible<V>) {
        utility::destruct<V>(values_data + i);
      }
    }
  }

  m_allocator->deallocate(reinterpret_cast<rx_byte*>(keys_data));
  m_allocator->deallocate(reinterpret_cast<rx_byte*>(values_data));
  m_allocator->deallocate(reinterpret_cast<rx_byte*>(hashes_data));
}

template<typename K, typename V>
inline V* map<K, V>::construct(rx_size _index, rx_size _hash, K&& key_, V&& value_) {
  utility::construct<K>(m_keys + _index, utility::forward<K>(key_));
  utility::construct<V>(m_values + _index, utility::forward<V>(value_));
  element_hash(_index) = _hash;
  return m_values + _index;
}

template<typename K, typename V>
inline V* map<K, V>::inserter(rx_size _hash, K&& key_, V&& value_) {
  rx_size position{desired_position(_hash)};
  rx_size distance{0};
  for (;;) {
    if (element_hash(position) == 0) {
      return construct(position, _hash, utility::forward<K>(key_), utility::forward<V>(value_));
    }

    const rx_size existing_element_probe_distance{probe_distance(element_hash(position), position)};
    if (existing_element_probe_distance < distance) {
      if (is_deleted(element_hash(position))) {
        return construct(position, _hash, utility::forward<K>(key_), utility::forward<V>(value_));
      }

      utility::swap(_hash, element_hash(position));
      utility::swap(key_, m_keys[position]);
      utility::swap(value_, m_values[position]);

      distance = existing_element_probe_distance;
    }

    position = (position + 1) & m_mask;
    distance++;
  }

  RX_HINT_UNREACHABLE();
}

template<typename K, typename V>
inline V* map<K, V>::inserter(rx_size _hash, const K& _key, V&& value_) {
  K key{_key};
  return inserter(_hash, utility::move(key), utility::forward<V>(value_));
}

template<typename K, typename V>
inline V* map<K, V>::inserter(rx_size _hash, const K& _key, const V& _value) {
  K key{_key};
  V value{_value};
  return inserter(_hash, utility::move(key), utility::move(value));
}

template<typename K, typename V>
inline bool map<K, V>::lookup_index(const K& _key, rx_size& _index) const {
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

template<typename K, typename V>
template<typename F>
inline bool map<K, V>::each_key(F&& _function) {
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

template<typename K, typename V>
template<typename F>
inline bool map<K, V>::each_key(F&& _function) const {
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

template<typename K, typename V>
template<typename F>
inline bool map<K, V>::each_value(F&& _function) {
 for (rx_size i{0}; i < m_capacity; i++) {
    const auto hash{m_hashes[i]};
    if (hash != 0 && !is_deleted(hash)) {
      if constexpr (traits::is_same<traits::return_type<F>, bool>) {
        if (!_function(m_values[i])) {
          return false;
        }
      } else {
        _function(m_values[i]);
      }
    }
  }
  return true;
}

template<typename K, typename V>
template<typename F>
inline bool map<K, V>::each_value(F&& _function) const {
 for (rx_size i{0}; i < m_capacity; i++) {
    const auto hash{m_hashes[i]};
    if (hash != 0 && !is_deleted(hash)) {
      if constexpr (traits::is_same<traits::return_type<F>, bool>) {
        if (!_function(m_values[i])) {
          return false;
        }
      } else {
        _function(m_values[i]);
      }
    }
  }
  return true;
}

template<typename K, typename V>
template<typename F>
inline bool map<K, V>::each_pair(F&& _function) {
  for (rx_size i{0}; i < m_capacity; i++) {
    const auto hash{m_hashes[i]};
    if (hash != 0 && !is_deleted(hash)) {
      if constexpr (traits::is_same<traits::return_type<F>, bool>) {
        if (!_function(m_keys[i], m_values[i])) {
          return false;
        }
      } else {
        _function(m_keys[i], m_values[i]);
      }
    }
  }
  return true;
}

template<typename K, typename V>
template<typename F>
inline bool map<K, V>::each_pair(F&& _function) const {
  for (rx_size i{0}; i < m_capacity; i++) {
    const auto hash{m_hashes[i]};
    if (hash != 0 && !is_deleted(hash)) {
      if constexpr (traits::is_same<traits::return_type<F>, bool>) {
        if (!_function(m_keys[i], m_values[i])) {
          return false;
        }
      } else {
        _function(m_keys[i], m_values[i]);
      }
    }
  }
  return true;
}

template<typename K, typename V>
inline memory::allocator* map<K, V>::allocator() const {
  return m_allocator;
}

} // namespace rx

#endif // RX_CORE_MAP
