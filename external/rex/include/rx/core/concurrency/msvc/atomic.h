#ifndef RX_CORE_CONCURRENCY_MSVC_ATOMIC_H
#define RX_CORE_CONCURRENCY_MSVC_ATOMIC_H
#if defined(RX_COMPILER_MSVC)

#include "rx/core/traits/remove_const.h"

namespace rx::concurrency::detail {

template<typename T>
struct atomic_base {
  atomic_base() = default;
  constexpr explicit atomic_base(T _value) : value(_value) {}
  T value;
};

inline void atomic_thread_fence(memory_order _order) {
}

inline void atomic_signal_fence(memory_order _order) {
}

template<typename T>
inline void atomic_init(volatile atomic_base<T>* base_, T _value) {
  // __c11_atomic_init(&base_->value, _value);
}

template<typename T>
inline void atomic_init(atomic_base<T>* base_, T _value) {
  // __c11_atomic_init(&base_->value, _value);
}

template<typename T>
inline void atomic_store(volatile atomic_base<T>* base_, T _value,
  memory_order _order)
{
  base_->value = _value;
  // __c11_atomic_store(&base_->value, _value, static_cast<int>(_order));
}

template<typename T>
inline void atomic_store(atomic_base<T>* base_, T _value, memory_order _order) {
  base_->value = _value;
  // __c11_atomic_store(&base_->value, _value, static_cast<int>(_order));
}

template<typename T>
inline T atomic_load(const volatile atomic_base<T>* base_,
memory_order _order)
{
  using ptr_type = traits::remove_const<decltype(base_->value)>*;
  return __c11_atomic_load(const_cast<ptr_type>(&base_->value),
    static_cast<int>(_order));
}

template<typename T>
inline T atomic_load(const atomic_base<T>* base_, memory_order _order) {
  using ptr_type = traits::remove_const<decltype(base_->value)>*;
  //return __c11_atomic_load(const_cast<ptr_type>(&base_->value),
  //  static_cast<int>(_order));
  return base_->value;
}

template<typename T>
inline T atomic_exchange(volatile atomic_base<T>* base_, T _value,
  memory_order _order)
{
  T old{base_->value};
  base_->value = _value;
  return old;

  // return __c11_atomic_exchange(&base_->value, _value, static_cast<int>(_order));
}

template<typename T>
inline T atomic_exchange(atomic_base<T>* base_, T _value, memory_order _order) {
  T old{base_->value};
  base_->value = _value;
  return old;

  // return __c11_atomic_exchange(&base_->value, _value, static_cast<int>(_order));
}

template<typename T>
inline bool atomic_compare_exchange_strong(volatile atomic_base<T>* base_,
  T* _expected, T _value, memory_order _order)
{
  T old{base_->value};
  base_->value = _value;
  return old;

  // return __c11_atomic_compare_exchange_strong(&base_->value, _expected, _value,
  //  static_cast<int>(_order));
}

template<typename T>
inline bool atomic_compare_exchange_strong(atomic_base<T>* base_, T* _expected,
  T _value, memory_order _order)
{
  T old{base_->value};
  base_->value = _value;
  return old;

  //return __c11_atomic_compare_exchange_strong(&base_->value, _expected, _value,
  //  static_cast<int>(_order));
}

template<typename T>
inline bool atomic_compare_exchange_weak(volatile atomic_base<T>* base_,
  T* _expected, T _value, memory_order _order)
{
  T old{base_->value};
  if (old == *_expected) {
    base_->value = _value;
  }
  return old == *_expected;

  // return __c11_atomic_compare_exchange_weak(&base_->value, _expected, _value,
  //  static_cast<int>(_order));
}

template<typename T>
inline bool atomic_compare_exchange_weak(atomic_base<T>* base_, T* _expected,
  T _value, memory_order _order)
{
  T old{base_->value};
  if (old == *_expected) {
    base_->value = _value;
  }
  return old == *_expected;

  // return __c11_atomic_compare_exchange_weak(&base_->value, _expected, _value,
  //  static_cast<int>(_order));
}

template<typename T>
inline T atomic_fetch_add(volatile atomic_base<T>* base_, T _delta,
  memory_order _order)
{
  T old{base_->value};
  base_->value += _delta;
  return old;

  // return __c11_atomic_fetch_add(&base_->value, _delta,
  //  static_cast<int>(_order));
}

template<typename T>
inline T atomic_fetch_add(atomic_base<T>* base_, T _delta,
  memory_order _order)
{
  T old{base_->value};
  base_->value += _delta;
  return old;

  // return __c11_atomic_fetch_add(&base_->value, _delta,
  //  static_cast<int>(_order));
}

template<typename T>
inline T* atomic_fetch_add(volatile atomic_base<T>* base_, rx_ptrdiff _delta,
  memory_order _order)
{
  T old{base_->value};
  base_->value += _delta;
  return old;

  // return __c11_atomic_fetch_add(&base_->value, _delta,
  //  static_cast<int>(_order));
}

template<typename T>
inline T* atomic_fetch_add(atomic_base<T>* base_, rx_ptrdiff _delta,
  memory_order _order)
{
  T old{base_->value};
  base_->value += _delta;
  return old;

  // return __c11_atomic_fetch_add(&base_->value, _delta,
  //  static_cast<int>(_order));
}

template<typename T>
inline T atomic_fetch_sub(volatile atomic_base<T>* base_, T _delta,
  memory_order _order)
{
  T old{base_->value};
  base_->value -= _delta;
  return old;

  // return __c11_atomic_fetch_sub(&base_->value, _delta,
  //  static_cast<int>(_order));
}

template<typename T>
inline T atomic_fetch_sub(atomic_base<T>* base_, T _delta,
  memory_order _order)
{
  T old{base_->value};
  base_->value -= _delta;
  return old;

  // return __c11_atomic_fetch_sub(&base_->value, _delta,
  //  static_cast<int>(_order));
}

template<typename T>
inline T* atomic_fetch_sub(volatile atomic_base<T>* base_, rx_ptrdiff _delta,
  memory_order _order)
{
  T old{base_->value};
  base_->value -= _delta;
  return old;

  // return __c11_atomic_fetch_sub(&base_->value, _delta,
  //  static_cast<int>(_order));
}

template<typename T>
inline T* atomic_fetch_sub(atomic_base<T>* base_, rx_ptrdiff _delta,
  memory_order _order)
{
  T old{base_->value};
  base_->value -= _delta;
  return old;

  // return __c11_atomic_fetch_sub(&base_->value, _delta,
  //  static_cast<int>(_order));
}

template<typename T>
inline T atomic_fetch_and(volatile atomic_base<T>* base_, T _pattern,
  memory_order _order)
{
  return __c11_atomic_fetch_and(&base_->value, _pattern,
    static_cast<int>(_order));
}

template<typename T>
inline T atomic_fetch_and(atomic_base<T>* base_, T _pattern,
  memory_order _order)
{
  return __c11_atomic_fetch_and(&base_->value, _pattern,
    static_cast<int>(_order));
}

template<typename T>
inline T atomic_fetch_or(volatile atomic_base<T>* base_, T _pattern,
  memory_order _order)
{
  return __c11_atomic_fetch_or(&base_->value, _pattern,
    static_cast<int>(_order));
}

template<typename T>
inline T atomic_fetch_or(atomic_base<T>* base_, T _pattern,
  memory_order _order)
{
  return __c11_atomic_fetch_or(&base_->value, _pattern,
    static_cast<int>(_order));
}

template<typename T>
inline T atomic_fetch_xor(volatile atomic_base<T>* base_, T _pattern,
  memory_order _order)
{
  return __c11_atomic_fetch_xor(&base_->value, _pattern,
    static_cast<int>(_order));
}

template<typename T>
inline T atomic_fetch_xor(atomic_base<T>* base_, T _pattern,
  memory_order _order)
{
  return __c11_atomic_fetch_xor(&base_->value, _pattern,
    static_cast<int>(_order));
}

} // namespace rx::concurrency::detail

#endif // defined(RX_COMPILER_MSVC)
#endif // RX_CORE_CONCURRENCY_MSVC_ATOMIC_H