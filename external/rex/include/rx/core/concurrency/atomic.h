#ifndef RX_CORE_CONCURRENCY_ATOMIC_H
#define RX_CORE_CONCURRENCY_ATOMIC_H
#include "rx/core/config.h" // RX_COMPILER_*
#include "rx/core/types.h"

#include "rx/core/traits/is_integral.h"
#include "rx/core/traits/is_same.h"

namespace rx::concurrency {

enum class memory_order {
  k_relaxed,
  k_consume, // load-consume
  k_acquire, // load-acquire
  k_release, // store-release
  k_acq_rel, // store-release load-acquire
  k_seq_cst  // store-release load-acquire
};

} // namespace rx::concurrency

#if defined(RX_COMPILER_GCC)
#include "rx/core/concurrency/gcc/atomic.h"
#elif defined(RX_COMPILER_CLANG)
#include "rx/core/concurrency/clang/atomic.h"
#elif defined(RX_COMPILER_MSVC)
#include "rx/core/concurrency/msvc/atomic.h"
#else
#error "missing atomic implementation"
#endif

namespace rx::concurrency {

namespace detail {
  template<typename T>
  struct atomic_value : atomic_base<T> {
    atomic_value() = default;
    constexpr explicit atomic_value(T _value) : atomic_base<T>{_value} {}
  };

  template<typename T, bool = traits::is_integral<T> && !traits::is_same<T, bool>>
  struct atomic {
    atomic() = default;
    constexpr atomic(T _value) : m_value{_value} {}

    atomic(const atomic&) = delete;
    atomic& operator=(const atomic&) = delete;
    atomic& operator=(const atomic&) volatile = delete;

    void store(T _value, memory_order _order = memory_order::k_seq_cst) volatile {
      atomic_store(&m_value, _value, _order);
    }

    void store(T _value, memory_order _order = memory_order::k_seq_cst) {
      atomic_store(&m_value, _value, _order);
    }

    T load(memory_order _order = memory_order::k_seq_cst) const volatile {
      return atomic_load(&m_value, _order);
    }

    T load(memory_order _order = memory_order::k_seq_cst) const {
      return atomic_load(&m_value, _order);
    }

    operator T() const volatile {
      return load();
    }

    operator T() const {
      return load();
    }

    T exchange(T _value, memory_order _order = memory_order::k_seq_cst) volatile {
      return atomic_exchange(&m_value, _value, _order);
    }

    T exchange(T _value, memory_order _order = memory_order::k_seq_cst) {
      return atomic_exchange(&m_value, _value, _order);
    }

    bool compare_exchange_weak(T& expected_, T _value, memory_order _success,
      memory_order _failure) volatile
    {
      return atomic_compare_exchange_weak(&m_value, expected_, _value, _success, _failure);
    }

    bool compare_exchange_weak(T& expected_, T _value, memory_order _success,
      memory_order _failure)
    {
      return atomic_compare_exchange_weak(&m_value, expected_, _value, _success, _failure);
    }

    bool compare_exchange_strong(T& expected_, T _value, memory_order _success,
      memory_order _failure) volatile
    {
      return atomic_compare_exchange_strong(&m_value, expected_, _value, _success, _failure);
    }

    bool compare_exchange_strong(T& expected_, T _value, memory_order _success,
      memory_order _failure)
    {
      return atomic_compare_exchange_strong(&m_value, expected_, _value, _success, _failure);
    }

    bool compare_exchange_weak(T& expected_, T _value, memory_order _order = memory_order::k_seq_cst) volatile {
      return atomic_compare_exchange_weak(&m_value, expected_, _value, _order, _order);
    }

    bool compare_exchange_weak(T& expected_, T _value, memory_order _order = memory_order::k_seq_cst) {
      return atomic_compare_exchange_weak(&m_value, expected_, _value, _order, _order);
    }

    bool compare_exchange_strong(T& expected_, T _value, memory_order _order) volatile {
      return atomic_compare_exchange_strong(&m_value, expected_, _value, _order, _order);
    }

    bool compare_exchange_strong(T& expected_, T _value, memory_order _order) {
      return atomic_compare_exchange_strong(&m_value, expected_, _value, _order, _order);
    }

  protected:
    mutable atomic_value<T> m_value;
  };

  // specialization for integral
  template<typename T>
  struct atomic<T, true> : atomic<T, false> {
    using base = atomic<T, false>;
    atomic() = default;
    constexpr atomic(T _value) : base{_value} {}

    T fetch_add(T _delta, memory_order _order = memory_order::k_seq_cst) volatile {
      return atomic_fetch_add(&this->m_value, _delta, _order);
    }

    T fetch_add(T _delta, memory_order _order = memory_order::k_seq_cst) {
      return atomic_fetch_add(&this->m_value, _delta, _order);
    }

    T fetch_sub(T _delta, memory_order _order = memory_order::k_seq_cst) volatile {
      return atomic_fetch_sub(&this->m_value, _delta, _order);
    }

    T fetch_sub(T _delta, memory_order _order = memory_order::k_seq_cst) {
      return atomic_fetch_sub(&this->m_value, _delta, _order);
    }

    T fetch_and(T _pattern, memory_order _order = memory_order::k_seq_cst) volatile {
      return atomic_fetch_and(&this->m_value, _pattern, _order);
    }

    T fetch_and(T _pattern, memory_order _order = memory_order::k_seq_cst) {
      return atomic_fetch_sub(&this->m_value, _pattern, _order);
    }

    T fetch_or(T _pattern, memory_order _order = memory_order::k_seq_cst) volatile {
      return atomic_fetch_or(&this->m_value, _pattern, _order);
    }

    T fetch_or(T _pattern, memory_order _order = memory_order::k_seq_cst) {
      return atomic_fetch_or(&this->m_value, _pattern, _order);
    }

    T fetch_xor(T _pattern, memory_order _order = memory_order::k_seq_cst) volatile {
      return atomic_fetch_xor(&this->m_value, _pattern, _order);
    }

    T fetch_xor(T _pattern, memory_order _order = memory_order::k_seq_cst) {
      return atomic_fetch_xor(&this->m_value, _pattern, _order);
    }

    // ++, --
    T operator++(int) volatile {
      return fetch_add(T{1});
    }

    T operator++(int) {
      return fetch_add(T{1});
    }

    T operator--(int) volatile {
      return fetch_sub(T{1});
    }

    T operator--(int) {
      return fetch_sub(T{1});
    }

    T operator++() volatile {
      return fetch_add(T{1}) + T{1};
    }

    T operator++() {
      return fetch_add(T{1}) + T{1};
    }

    T operator--() volatile {
      return fetch_sub(T{1}) - T{1};
    }

    T operator--() {
      return fetch_sub(T{1}) - T{1};
    }

    T operator+=(T _delta) volatile {
      return fetch_add(_delta) + _delta;
    }

    T operator+=(T _delta) {
      return fetch_add(_delta) + _delta;
    }

    T operator-=(T _delta) volatile {
      return fetch_sub(_delta) - _delta;
    }

    T operator-=(T _delta) {
      return fetch_sub(_delta) - _delta;
    }

    T operator&=(T _pattern) volatile {
      return fetch_and(_pattern) & _pattern;
    }

    T operator&=(T _pattern) {
      return fetch_and(_pattern) & _pattern;
    }

    T operator|=(T _pattern) volatile {
      return fetch_or(_pattern) | _pattern;
    }

    T operator|=(T _pattern) {
      return fetch_or(_pattern) | _pattern;
    }

    T operator^=(T _pattern) volatile {
      return fetch_xor(_pattern) ^ _pattern;
    }

    T operator^=(T _pattern) {
      return fetch_xor(_pattern) ^ _pattern;
    }
  };

} // namespace detail

template<typename T>
struct atomic : detail::atomic<T> {
  using base = detail::atomic<T>;
  atomic() = default;
  constexpr atomic(T _value) : base{_value} {}

  T operator=(T _value) volatile {
    base::store(_value);
    return _value;
  }

  T operator=(T _value) {
    base::store(_value);
    return _value;
  }
};

template<typename T>
struct atomic<T*> : detail::atomic<T*> {
  using base = detail::atomic<T*>;

  atomic() = default;
  constexpr atomic(T* _value) : base{_value} {}

  T* operator=(T* _value) volatile {
    base::store(_value);
    return _value;
  }

  T* operator=(T* _value) {
    base::store(_value);
    return _value;
  }

  T* fetch_add(rx_ptrdiff _delta, memory_order _order = memory_order::k_seq_cst) volatile {
    return detail::atomic_fetch_add(&this->m_value, _delta, _order);
  }

  T* fetch_add(rx_ptrdiff _delta, memory_order _order = memory_order::k_seq_cst) {
    return detail::atomic_fetch_add(&this->m_value, _delta, _order);
  }

  T* fetch_sub(rx_ptrdiff _delta, memory_order _order = memory_order::k_seq_cst) volatile {
    return detail::atomic_fetch_sub(&this->m_value, _delta, _order);
  }

  T* fetch_sub(rx_ptrdiff _delta, memory_order _order = memory_order::k_seq_cst) {
    return detail::atomic_fetch_sub(&this->m_value, _delta, _order);
  }

  T* operator++(int) volatile {
    return fetch_add(1);
  }

  T* operator++(int) {
    return fetch_add(1);
  }

  T* operator--(int) volatile {
    return fetch_sub(1);
  }

  T* operator--(int) {
    return fetch_sub(1);
  }

  T* operator++() volatile {
    return fetch_add(1) + 1;
  }

  T* operator++() {
    return fetch_add(1) + 1;
  }

  T* operator--() volatile {
    return fetch_sub(1) - 1;
  }

  T* operator--() {
    return fetch_sub(1) - 1;
  }

  T* operator+=(rx_ptrdiff _delta) volatile {
    return fetch_add(_delta) + _delta;
  }

  T* operator+=(rx_ptrdiff _delta) {
    return fetch_add(_delta) + _delta;
  }

  T* operator-=(rx_ptrdiff _delta) volatile {
    return fetch_sub(_delta) - _delta;
  }

  T* operator-=(rx_ptrdiff _delta) {
    return fetch_sub(_delta) - _delta;
  }
};

struct atomic_flag {
  atomic_flag() = default;
  constexpr atomic_flag(bool _value) : m_value{_value} {}

  atomic_flag(const atomic_flag&) = delete;
  atomic_flag& operator=(const atomic_flag&) = delete;
  atomic_flag& operator=(const atomic_flag&) volatile = delete;

  bool test_and_set(memory_order _order = memory_order::k_seq_cst) volatile {
    return detail::atomic_exchange(&m_value, true, _order);
  }

  bool test_and_set(memory_order _order = memory_order::k_seq_cst)  {
    return detail::atomic_exchange(&m_value, true, _order);
  }

  void clear(memory_order _order = memory_order::k_seq_cst) volatile {
    detail::atomic_store(&m_value, false, _order);
  }

  void clear(memory_order _order = memory_order::k_seq_cst) {
    detail::atomic_store(&m_value, false, _order);
  }

private:
  detail::atomic_base<bool> m_value;
};

} // namespace rx::concurrency

#endif // RX_CORE_CONCURRENCY_ATOMIC_H
