#ifndef RX_CORE_ARRAY_H
#define RX_CORE_ARRAY_H
#include "rx/core/types.h"
#include "rx/core/assert.h"

namespace rx {

template<typename T>
struct array;

template<typename T, rx_size E>
struct array<T[E]> {
  constexpr T& operator[](rx_size _index);
  constexpr const T& operator[](rx_size _index) const;

  constexpr T* data();
  constexpr const T* data() const;
  constexpr rx_size size() const;

  T m_data[E];
};

template<typename T, rx_size E>
inline constexpr T& array<T[E]>::operator[](rx_size _index) {
  RX_ASSERT(_index < E, "out of bounds (%zu >= %zu)", _index, E);
  return m_data[_index];
}

template<typename T, rx_size E>
inline constexpr const T& array<T[E]>::operator[](rx_size _index) const {
  RX_ASSERT(_index < E, "out of bounds (%zu >= %zu)", _index, E);
  return m_data[_index];
}

template<typename T, rx_size E>
inline constexpr T* array<T[E]>::data() {
  return m_data;
}

template<typename T, rx_size E>
inline constexpr const T* array<T[E]>::data() const {
  return m_data;
}

template<typename T, rx_size E>
inline constexpr rx_size array<T[E]>::size() const {
  return E;
}

} // namespace rx

#endif // RX_CORE_ARRAY_H
