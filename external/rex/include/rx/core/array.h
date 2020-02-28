#ifndef RX_CORE_ARRAY_H
#define RX_CORE_ARRAY_H
#include "rx/core/types.h"
#include "rx/core/assert.h"

#include "rx/core/utility/forward.h"

namespace rx {

template<typename T>
struct array;

template<typename T, rx_size E>
struct array<T[E]> {
  template<typename... Ts>
  constexpr array(Ts&&... _arguments);

  constexpr T& operator[](rx_size _index);
  constexpr const T& operator[](rx_size _index) const;

  constexpr T* data();
  constexpr const T* data() const;

  constexpr rx_size size() const;

private:
  T m_data[E];
};

// Deduction guide for array{Ts...} to become array<T[E]>.
template<typename T, typename... Ts>
array(T, Ts...) -> array<T[1 + sizeof...(Ts)]>;

template<typename T, rx_size E>
template<typename... Ts>
inline constexpr array<T[E]>::array(Ts&&... _arguments)
  : m_data{utility::forward<Ts>(_arguments)...}
{
}

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
