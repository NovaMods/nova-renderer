#ifndef RX_CORE_CONCURRENCY_GCC_ATOMIC_H
#define RX_CORE_CONCURRENCY_GCC_ATOMIC_H
#if defined(RX_COMPILER_GCC)

#include "rx/core/traits/enable_if.h"
#include "rx/core/traits/is_assignable.h"

namespace rx::concurrency::detail {

static constexpr int to_success_order(memory_order _order) {
  return _order == memory_order::k_relaxed ? __ATOMIC_RELAXED :
          (_order == memory_order::k_acquire ? __ATOMIC_ACQUIRE :
            (_order == memory_order::k_release ? __ATOMIC_RELEASE :
              (_order == memory_order::k_seq_cst ? __ATOMIC_SEQ_CST :
                (_order == memory_order::k_acq_rel ? __ATOMIC_ACQ_REL :
                  __ATOMIC_CONSUME))));
}

static constexpr int to_failure_order(memory_order _order) {
  return _order == memory_order::k_relaxed ? __ATOMIC_RELAXED :
          (_order == memory_order::k_acquire ? __ATOMIC_ACQUIRE :
            (_order == memory_order::k_release ? __ATOMIC_RELAXED :
              (_order == memory_order::k_seq_cst ? __ATOMIC_SEQ_CST :
                (_order == memory_order::k_acq_rel ? __ATOMIC_ACQUIRE :
                  __ATOMIC_CONSUME))));
}

template<typename Tp, typename Tv>
inline traits::enable_if<traits::is_assignable<Tp&, Tv>>
atomic_assign_volatile(Tp& value_, const Tv& _value) {
  value_ = _value;
}

template<typename Tp, typename Tv>
inline traits::enable_if<traits::is_assignable<Tp&, Tv>>
atomic_assign_volatile(volatile Tp& value_, const volatile Tv& _value) {
  volatile char* to{reinterpret_cast<volatile char*>(&value_)};
  volatile char* end{to + sizeof(Tp)};
  volatile const char* from{reinterpret_cast<volatile const char*>(&_value)};
  while (to != end) {
    *to++ = *from++;
  }
}

template<typename T>
struct atomic_base {
  atomic_base() = default;
  constexpr explicit atomic_base(T _value) : value(_value) {}
  T value;
};

template<typename T>
inline void atomic_init(volatile atomic_base<T>* base_, T _value) {
  atomic_assign_volatile(base_->value, _value);
}

template<typename T>
inline void atomic_init(atomic_base<T>* base_, T _value) {
  base_->value = _value;
}

inline void atomic_thread_fence(memory_order _order) {
  __atomic_thread_fence(to_success_order(_order));
}

inline void atomic_signal_fence(memory_order _order) {
  __atomic_signal_fence(to_success_order(_order));
}

template<typename T>
inline void atomic_store(volatile atomic_base<T>* base_, T _value,
  memory_order _order)
{
  __atomic_store(&base_->value, &_value, to_success_order(_order));
}

template<typename T>
inline void atomic_store(atomic_base<T>* base_, T _value, memory_order _order) {
  __atomic_store(&base_->value, &_value, to_success_order(_order));
}

template<typename T>
inline T atomic_load(const volatile atomic_base<T>* _base,
  memory_order _order)
{
  T result;
  __atomic_load(&_base->value, &result, to_success_order(_order));
  return result;
}

template<typename T>
inline T atomic_load(const atomic_base<T>* _base, memory_order _order) {
  T result;
  __atomic_load(&_base->value, &result, to_success_order(_order));
  return result;
}

template<typename T>
inline T atomic_exchange(volatile atomic_base<T>* base_, T _value,
  memory_order _order)
{
  T result;
  __atomic_exchange(&base_->value, &_value, &result, to_success_order(_order));
  return result;
}

template<typename T>
inline T atomic_exchange(atomic_base<T>* base_, T _value, memory_order _order) {
  T result;
  __atomic_exchange(&base_->value, &_value, &result, to_success_order(_order));
  return result;
}

template<typename T>
inline bool atomic_compare_exchange_strong(volatile atomic_base<T>* base_,
  T* _expected, T _value, memory_order _success, memory_order _failure)
{
  return __atomic_compare_exchange(&base_->value, _expected, &_value, false,
    to_success_order(_success), to_failure_order(_failure));
}

template<typename T>
inline bool atomic_compare_exchange_strong(atomic_base<T>* base_,
  T* _expected, T _value, memory_order _success, memory_order _failure)
{
  return __atomic_compare_exchange(&base_->value, _expected, &_value, false,
    to_success_order(_success), to_failure_order(_failure));
}

template<typename T>
inline bool atomic_compare_exchange_weak(volatile atomic_base<T>* base_,
  T* _expected, T _value, memory_order _success, memory_order _failure)
{
  return __atomic_compare_exchange(&base_->value, _expected, &_value, true,
    to_success_order(_success), to_failure_order(_failure));
}

template<typename T>
inline bool atomic_compare_exchange_weak(atomic_base<T>* base_,
  T* _expected, T _value, memory_order _success, memory_order _failure)
{
  return __atomic_compare_exchange(&base_->value, _expected, &_value, true,
    to_success_order(_success), to_failure_order(_failure));
}

template<typename T>
inline constexpr const auto k_fetch_skip{1};

template<typename T>
inline constexpr const auto k_fetch_skip<T*>{sizeof(T)};

template<typename Tp, typename Td>
inline Tp atomic_fetch_add(volatile atomic_base<Tp>* base_, Td _delta,
  memory_order _order)
{
  return __atomic_fetch_add(&base_->value, _delta * k_fetch_skip<Tp>,
    to_success_order(_order));
}

template<typename Tp, typename Td>
inline Tp atomic_fetch_add(atomic_base<Tp>* base_, Td _delta,
  memory_order _order)
{
  return __atomic_fetch_add(&base_->value, _delta * k_fetch_skip<Tp>,
    to_success_order(_order));
}

template<typename Tp, typename Td>
inline Tp atomic_fetch_sub(volatile atomic_base<Tp>* base_, Td _delta,
  memory_order _order)
{
  return __atomic_fetch_sub(&base_->value, _delta * k_fetch_skip<Tp>,
    to_success_order(_order));
}

template<typename Tp, typename Td>
inline Tp atomic_fetch_sub(atomic_base<Tp>* base_, Td _delta,
  memory_order _order)
{
  return __atomic_fetch_sub(&base_->value, _delta * k_fetch_skip<Tp>,
    to_success_order(_order));
}

template<typename T>
inline T atomic_fetch_and(volatile atomic_base<T>* base_, T _pattern,
  memory_order _order)
{
  return __atomic_fetch_and(&base_->value, _pattern, to_success_order(_order));
}

template<typename T>
inline T atomic_fetch_and(atomic_base<T>* base_, T _pattern,
  memory_order _order)
{
  return __atomic_fetch_and(&base_->value, _pattern, to_success_order(_order));
}

template<typename T>
inline T atomic_fetch_or(volatile atomic_base<T>* base_, T _pattern,
  memory_order _order)
{
  return __atomic_fetch_or(&base_->value, _pattern, to_success_order(_order));
}

template<typename T>
inline T atomic_fetch_or(atomic_base<T>* base_, T _pattern,
  memory_order _order)
{
  return __atomic_fetch_or(&base_->value, _pattern, to_success_order(_order));
}

template<typename T>
inline T atomic_fetch_xor(volatile atomic_base<T>* base_, T _pattern,
  memory_order _order)
{
  return __atomic_fetch_oxr(&base_->value, _pattern, to_success_order(_order));
}

template<typename T>
inline T atomic_fetch_xor(atomic_base<T>* base_, T _pattern,
  memory_order _order)
{
  return __atomic_fetch_xor(&base_->value, _pattern, to_success_order(_order));
}

} // namespace rx::concurrency::detail

#endif // defined(RX_COMPILER_GCC)
#endif // RX_CORE_CONCURRENCY_GCC_ATOMIC_H