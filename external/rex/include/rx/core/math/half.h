#ifndef RX_CORE_MATH_HALF_H
#define RX_CORE_MATH_HALF_H
#include "rx/core/types.h"
#include "rx/core/assert.h" // RX_ASSERT

namespace rx::math {

struct half {
  constexpr half(const half& _h);
  constexpr half& operator=(const half& _h);

  half(rx_f32 _f);
  half(rx_f64 _f);

  rx_f32 to_f32() const;
  rx_f64 to_f64() const;

  friend half operator+(half _lhs, half _rhs);
  friend half operator-(half _lhs, half _rhs);
  friend half operator*(half _lhs, half _rhs);
  friend half operator/(half _lhs, half _rhs);

  friend half& operator+=(half& _lhs, half _rhs);
  friend half& operator-=(half& _lhs, half _rhs);
  friend half& operator*=(half& _lhs, half _rhs);
  friend half& operator/=(half& _lhs, half _rhs);

  friend half operator-(half _h);
  friend half operator+(half _h);

private:
  constexpr explicit half(rx_u16 _bits);

  half to_half(rx_f32 _f);
  rx_u16 m_bits;
};

inline constexpr half::half(const half& _h)
  : m_bits{_h.m_bits}
{
}

inline constexpr half& half::operator=(const half& _h) {
  RX_ASSERT(&_h != this, "self assignment");

  m_bits = _h.m_bits;
  return *this;
}

inline half::half(rx_f32 _f)
  : half{to_half(_f)}
{
}

inline half::half(rx_f64 _f)
  : half{static_cast<rx_f32>(_f)}
{
}

inline rx_f64 half::to_f64() const {
  return static_cast<rx_f64>(to_f32());
}

inline half operator+(half _lhs, half _rhs) {
  return _lhs.to_f32() + _rhs.to_f32();
}

inline half operator-(half _lhs, half _rhs) {
  return _lhs.to_f32() - _rhs.to_f32();
}

inline half operator*(half _lhs, half _rhs) {
  return _lhs.to_f32() * _rhs.to_f32();
}

inline half operator/(half _lhs, half _rhs) {
  return _lhs.to_f32() / _rhs.to_f32();
}

inline half& operator+=(half& _lhs, half _rhs) {
  return _lhs = _lhs + _rhs;
}

inline half& operator-=(half& _lhs, half _rhs) {
  return _lhs = _lhs - _rhs;
}

inline half& operator*=(half& _lhs, half _rhs) {
  return _lhs = _lhs * _rhs;
}

inline half& operator/=(half& _lhs, half _rhs) {
  return _lhs = _lhs / _rhs;
}

inline half operator-(half _h) {
  return -_h.to_f32();
}

inline half operator+(half _h) {
  return +_h.to_f32();
}

inline constexpr half::half(rx_u16 _bits)
  : m_bits{_bits}
{
}

} // namespace rx::math

#endif // RX_CORE_MATH_HALF_H
