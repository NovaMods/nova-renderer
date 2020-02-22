#ifndef RX_CORE_VECTOR_H
#define RX_CORE_VECTOR_H
#include "rx/core/array.h"

#include "rx/core/traits/is_same.h"
#include "rx/core/traits/is_trivially_copyable.h"
#include "rx/core/traits/is_trivially_destructible.h"
#include "rx/core/traits/return_type.h"

#include "rx/core/utility/forward.h"
#include "rx/core/utility/move.h"
#include "rx/core/utility/uninitialized.h"

#include "rx/core/hints/restrict.h"

#include "rx/core/memory/system_allocator.h" // memory::{system_allocator, allocator}

namespace rx {

namespace detail {
  void copy(void *RX_HINT_RESTRICT dst_, const void* RX_HINT_RESTRICT _src, rx_size _size);
}

// 32-bit: 16 bytes
// 64-bit: 32 bytes
template<typename T>
struct vector {
  template<typename U, rx_size E>
  using initializers = array<U[E]>;

  static constexpr const rx_size k_npos{-1_z};

  constexpr vector();
  constexpr vector(memory::allocator* _allocator);
  constexpr vector(memory::view _view);

  // Construct a vector from an array of initializers. This is similar to
  // how initializer_list works in C++11 except it requires no compiler proxy
  // and is actually faster since the initializer type can be moved.
  template<typename U, rx_size E>
  vector(memory::allocator* _allocator, initializers<U, E>&& _initializers);
  template<typename U, rx_size E>
  vector(initializers<U, E>&& _initializers);

  vector(memory::allocator* _allocator, rx_size _size, utility::uninitialized);
  vector(memory::allocator* _allocator, rx_size _size);
  vector(memory::allocator* _allocator, const vector& _other);
  vector(rx_size _size);
  vector(const vector& _other);
  vector(vector&& other_);

  ~vector();

  vector& operator=(const vector& _other);
  vector& operator=(vector&& other_);

  vector& operator+=(const vector& _other);
  vector& operator+=(vector&& other_);

  T& operator[](rx_size _index);
  const T& operator[](rx_size _index) const;

  // resize to |size| with |value| for new objects
  bool resize(rx_size _size, const T& _value = {});

  // Resize of |_size| where the contents stays uninitialized.
  // This should only be used with trivially copyable T.
  bool resize(rx_size _size, utility::uninitialized);

  // reserve |size| elements
  bool reserve(rx_size _size);

  void clear();

  rx_size find(const T& _value) const;

  template<typename F>
  rx_size find_if(F&& _compare) const;

  // append |data| by copy
  bool push_back(const T& _data);
  // append |data| by move
  bool push_back(T&& data_);

  // append new |T| construct with |args|
  template<typename... Ts>
  bool emplace_back(Ts&&... _args);

  rx_size size() const;
  rx_size capacity() const;

  bool is_empty() const;

  // enumerate collection either forward or reverse
  template<typename F>
  bool each_fwd(F&& _func);
  template<typename F>
  bool each_rev(F&& _func);
  template<typename F>
  bool each_fwd(F&& _func) const;
  template<typename F>
  bool each_rev(F&& _func) const;

  void erase(rx_size _from, rx_size _to);

  // first or last element
  const T& first() const;
  T& first();
  const T& last() const;
  T& last();

  const T* data() const;
  T* data();

  memory::allocator* allocator() const;

  memory::view disown();

private:
  // NOTE(dweiler): This does not adjust m_size, it only adjusts capacity.
  bool grow_or_shrink_to(rx_size _size);

  memory::allocator* m_allocator;
  T* m_data;
  rx_size m_size;
  rx_size m_capacity;
};

template<typename T>
inline constexpr vector<T>::vector()
  : vector{&memory::g_system_allocator}
{
}

template<typename T>
inline constexpr vector<T>::vector(memory::allocator* _allocator)
  : m_allocator{_allocator}
  , m_data{nullptr}
  , m_size{0}
  , m_capacity{0}
{
  RX_ASSERT(m_allocator, "null allocator");
}

template<typename T>
inline constexpr vector<T>::vector(memory::view _view)
  : m_allocator{_view.owner}
  , m_data{reinterpret_cast<T*>(_view.data)}
  , m_size{_view.size}
  , m_capacity{m_size}
{
  RX_ASSERT(m_allocator, "null allocator");
}

template<typename T>
template<typename U, rx_size E>
inline vector<T>::vector(memory::allocator* _allocator, initializers<U, E>&& _initializers)
  : vector{_allocator}
{
  grow_or_shrink_to(E);
  if constexpr(traits::is_trivially_copyable<T>) {
    detail::copy(m_data, _initializers.data(), sizeof(T) * E);
  } else for (rx_size i = 0; i < E; i++) {
    utility::construct<T>(m_data + i, utility::move(_initializers[i]));
  }
  m_size = E;
}

template<typename T>
template<typename U, rx_size E>
inline vector<T>::vector(initializers<U, E>&& _initializers)
  : vector{&memory::g_system_allocator, utility::move(_initializers)}
{
}

template<typename T>
inline vector<T>::vector(memory::allocator* _allocator, rx_size _size, utility::uninitialized)
  : m_allocator{_allocator}
  , m_data{nullptr}
  , m_size{_size}
  , m_capacity{_size}
{
  RX_ASSERT(traits::is_trivially_copyable<T>,
    "T isn't trivial, cannot leave uninitialized");
  RX_ASSERT(m_allocator, "null allocator");

  m_data = reinterpret_cast<T*>(m_allocator->allocate(m_size * sizeof *m_data));
  RX_ASSERT(m_data, "out of memory");
}

template<typename T>
inline vector<T>::vector(memory::allocator* _allocator, rx_size _size)
  : m_allocator{_allocator}
  , m_data{nullptr}
  , m_size{_size}
  , m_capacity{_size}
{
  RX_ASSERT(m_allocator, "null allocator");

  m_data = reinterpret_cast<T*>(m_allocator->allocate(m_size * sizeof *m_data));

  RX_ASSERT(m_data, "out of memory");

  // TODO(dweiler): is_trivial trait so we can memset this.
  for (rx_size i = 0; i < m_size; i++) {
    utility::construct<T>(m_data + i);
  }
}

template<typename T>
inline vector<T>::vector(memory::allocator* _allocator, const vector& _other)
  : m_allocator{_allocator}
  , m_size{_other.m_size}
  , m_capacity{_other.m_capacity}
{
  RX_ASSERT(m_allocator, "null allocator");

  m_data = reinterpret_cast<T*>(m_allocator->allocate(_other.m_capacity * sizeof *m_data));
  RX_ASSERT(m_data, "out of memory");

  if constexpr(traits::is_trivially_copyable<T>) {
    detail::copy(m_data, _other.m_data, _other.m_size * sizeof *m_data);
  } else for (rx_size i{0}; i < m_size; i++) {
    utility::construct<T>(m_data + i, _other.m_data[i]);
  }
}

template<typename T>
inline vector<T>::vector(rx_size _size)
  : vector{&memory::g_system_allocator, _size}
{
}

template<typename T>
inline vector<T>::vector(const vector& _other)
  : vector{_other.m_allocator, _other}
{
}

template<typename T>
inline vector<T>::vector(vector&& other_)
  : m_allocator{other_.m_allocator}
  , m_data{other_.m_data}
  , m_size{other_.m_size}
  , m_capacity{other_.m_capacity}
{
  other_.m_allocator = &memory::g_system_allocator;
  other_.m_data = nullptr;
  other_.m_size = 0;
  other_.m_capacity = 0;
}

template<typename T>
inline vector<T>::~vector() {
  clear();
  m_allocator->deallocate(reinterpret_cast<rx_byte*>(m_data));
}

template<typename T>
inline vector<T>& vector<T>::operator=(const vector& _other) {
  RX_ASSERT(&_other != this, "self assignment");

  clear();
  m_allocator->deallocate(reinterpret_cast<rx_byte*>(m_data));

  m_allocator = _other.m_allocator;
  m_size = _other.m_size;
  m_capacity = _other.m_capacity;
  m_data = reinterpret_cast<T*>(m_allocator->allocate(_other.m_capacity * sizeof *m_data));
  RX_ASSERT(m_data, "out of memory");

  if constexpr(traits::is_trivially_copyable<T>) {
    detail::copy(m_data, _other.m_data, _other.m_size * sizeof *m_data);
  } else for (rx_size i = 0; i < m_size; i++) {
    utility::construct<T>(m_data + i, _other.m_data[i]);
  }

  return *this;
}

template<typename T>
inline vector<T>& vector<T>::operator=(vector&& other_) {
  RX_ASSERT(&other_ != this, "self assignment");

  clear();
  m_allocator->deallocate(reinterpret_cast<rx_byte*>(m_data));

  m_allocator = other_.m_allocator;
  m_data = other_.m_data;
  m_size = other_.m_size;
  m_capacity = other_.m_capacity;

  other_.m_data = nullptr;
  other_.m_size = 0;
  other_.m_capacity = 0;

  return *this;
}

template<typename T>
inline vector<T>& vector<T>::operator+=(const vector& _other) {
  reserve(size() + _other.size());
  _other.each_fwd([this](const T& _value) {
    push_back(_value);
  });
  return *this;
}

template<typename T>
inline vector<T>& vector<T>::operator+=(vector&& other_) {
  reserve(size() + other_.size());
  other_.each_fwd([this](T& value_) {
    push_back(utility::move(value_));
  });
  other_.clear();
  return *this;
}

template<typename T>
inline T& vector<T>::operator[](rx_size _index) {
  RX_ASSERT(_index < m_size, "out of bounds (%zu >= %zu)", _index, m_size);
  return m_data[_index];
}

template<typename T>
inline const T& vector<T>::operator[](rx_size _index) const {
  RX_ASSERT(_index < m_size, "out of bounds (%zu >= %zu)", _index, m_size);
  return m_data[_index];
}

template<typename T>
bool vector<T>::grow_or_shrink_to(rx_size _size) {
  if (!reserve(_size)) {
    return false;
  }

  if (_size < m_size) {
    if constexpr (!traits::is_trivially_destructible<T>) {
      for (rx_size i = m_size-1; i >= _size; i--) {
        utility::destruct<T>(m_data + i);
      }
    }
  }

  return true;
}

template<typename T>
bool vector<T>::resize(rx_size _size, const T& _value) {
  if (!grow_or_shrink_to(_size)) {
    return false;
  }

  // Copy construct the objects.
  for (rx_size i{m_size}; i < _size; i++) {
    if constexpr(traits::is_trivially_copyable<T>) {
      m_data[i] = _value;
    } else {
      utility::construct<T>(m_data + i, _value);
    }
  }

  m_size = _size;
  return true;
}

template<typename T>
bool vector<T>::resize(rx_size _size, utility::uninitialized) {
  RX_ASSERT(traits::is_trivially_copyable<T>,
    "T isn't trivial, cannot leave uninitialized");

  if (!grow_or_shrink_to(_size)) {
    return false;
  }

  m_size = _size;
  return true;
}

template<typename T>
bool vector<T>::reserve(rx_size _size) {
  if (_size <= m_capacity) {
    return true;
  }

  // golden ratio
  while (m_capacity < _size) {
    m_capacity = ((m_capacity + 1) * 3) / 2;
  }

  if constexpr (traits::is_trivially_copyable<T>) {
    T* resize{reinterpret_cast<T*>(m_allocator->reallocate(reinterpret_cast<rx_byte*>(m_data), m_capacity * sizeof *m_data))};
    if (resize) {
      m_data = resize;
      return true;
    }
  } else {
    T* resize{reinterpret_cast<T*>(m_allocator->allocate(m_capacity * sizeof *m_data))};
    if (resize) {
      for (rx_size i{0}; i < m_size; i++) {
        utility::construct<T>(resize + i, utility::move(*(m_data + i)));
        utility::destruct<T>(m_data + i);
      }

      m_allocator->deallocate(reinterpret_cast<rx_byte*>(m_data));
      m_data = resize;
      return true;
    }
  }

  return false;
}

template<typename T>
inline void vector<T>::clear() {
  if (m_size) {
    if constexpr (!traits::is_trivially_destructible<T>) {
      for (rx_size i = m_size-1; i < m_size; i--) {
        utility::destruct<T>(m_data + i);
      }
    }
  }
  m_size = 0;
}

template<typename T>
inline rx_size vector<T>::find(const T& _value) const {
  for (rx_size i{0}; i < m_size; i++) {
    if (m_data[i] == _value) {
      return i;
    }
  }
  return k_npos;
}

template<typename T>
template<typename F>
inline rx_size vector<T>::find_if(F&& _compare) const {
  for (rx_size i{0}; i < m_size; i++) {
    if (_compare(m_data[i])) {
      return i;
    }
  }
  return k_npos;
}

template<typename T>
inline bool vector<T>::push_back(const T& _value) {
  if (!grow_or_shrink_to(m_size + 1)) {
    return false;
  }

  // Copy construct object.
  utility::construct<T>(m_data + m_size, _value);

  m_size++;
  return true;
}

template<typename T>
inline bool vector<T>::push_back(T&& value_) {
  if (!grow_or_shrink_to(m_size + 1)) {
    return false;
  }

  // Move construct object.
  utility::construct<T>(m_data + m_size, utility::move(value_));

  m_size++;
  return true;
}

template<typename T>
template<typename... Ts>
inline bool vector<T>::emplace_back(Ts&&... _args) {
  if (!grow_or_shrink_to(m_size + 1)) {
    return false;
  }

  // Forward construct object.
  utility::construct<T>(m_data + m_size, utility::forward<Ts>(_args)...);

  m_size++;
  return true;
}

template<typename T>
RX_HINT_FORCE_INLINE rx_size vector<T>::size() const {
  return m_size;
}

template<typename T>
RX_HINT_FORCE_INLINE rx_size vector<T>::capacity() const {
  return m_capacity;
}

template<typename T>
RX_HINT_FORCE_INLINE bool vector<T>::is_empty() const {
  return m_size == 0;
}

template<typename T>
template<typename F>
inline bool vector<T>::each_fwd(F&& _func) {
  for (rx_size i{0}; i < m_size; i++) {
    if constexpr (traits::is_same<traits::return_type<F>, bool>) {
      if (!_func(m_data[i])) {
        return false;
      }
    } else {
      _func(m_data[i]);
    }
  }
  return true;
}

template<typename T>
template<typename F>
inline bool vector<T>::each_fwd(F&& _func) const {
  for (rx_size i{0}; i < m_size; i++) {
    if constexpr (traits::is_same<traits::return_type<F>, bool>) {
      if (!_func(m_data[i])) {
        return false;
      }
    } else {
      _func(m_data[i]);
    }
  }
  return true;
}

template<typename T>
template<typename F>
inline bool vector<T>::each_rev(F&& _func) {
  for (rx_size i{m_size-1}; i < m_size; i--) {
    if constexpr (traits::is_same<traits::return_type<F>, bool>) {
      if (!_func(m_data[i])) {
        return false;
      }
    } else {
      _func(m_data[i]);
    }
  }
  return true;
}

template<typename T>
inline void vector<T>::erase(rx_size _from, rx_size _to) {
  const rx_size range{_to-_from};
  T* begin{m_data};
  T* end{m_data + m_size};
  T* first{begin + _from};
  T* last{begin + _to};

  for (T* value{last}, *dest{first}; value != end; ++value, ++dest) {
    *dest = utility::move(*value);
  }

  if constexpr (!traits::is_trivially_destructible<T>) {
    for (T* value{end-range}; value < end; ++value) {
      utility::destruct<T>(value);
    }
  }

  m_size -= range;
}

template<typename T>
template<typename F>
inline bool vector<T>::each_rev(F&& _func) const {
  for (rx_size i{m_size-1}; i < m_size; i--) {
    if constexpr (traits::is_same<traits::return_type<F>, bool>) {
      if (!_func(m_data[i])) {
        return false;
      }
    } else {
      _func(m_data[i]);
    }
  }
  return true;
}

template<typename T>
RX_HINT_FORCE_INLINE const T& vector<T>::first() const {
  return m_data[0];
}

template<typename T>
RX_HINT_FORCE_INLINE T& vector<T>::first() {
  return m_data[0];
}

template<typename T>
RX_HINT_FORCE_INLINE const T& vector<T>::last() const {
  return m_data[m_size - 1];
}

template<typename T>
RX_HINT_FORCE_INLINE T& vector<T>::last() {
  return m_data[m_size - 1];
}

template<typename T>
RX_HINT_FORCE_INLINE const T* vector<T>::data() const {
  return m_data;
}

template<typename T>
RX_HINT_FORCE_INLINE T* vector<T>::data() {
  return m_data;
}

template<typename T>
RX_HINT_FORCE_INLINE memory::allocator* vector<T>::allocator() const {
  return m_allocator;
}

template<typename T>
inline memory::view vector<T>::disown() {
  memory::view view{allocator(), reinterpret_cast<rx_byte*>(data()), capacity()*sizeof(T)};
  m_data = nullptr;
  m_size = 0;
  m_capacity = 0;
  return view;
}

} // namespace rx

#endif // RX_CORE_VECTOR_H
