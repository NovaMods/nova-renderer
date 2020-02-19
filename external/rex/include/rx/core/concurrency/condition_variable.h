#ifndef RX_CORE_CONCURRENCY_CONDITION_VARIABLE_H
#define RX_CORE_CONCURRENCY_CONDITION_VARIABLE_H
#include "rx/core/concurrency/scope_lock.h" // scope_lock
#include "rx/core/concurrency/mutex.h" // mutex

namespace rx::concurrency {

struct condition_variable {
  condition_variable();
  ~condition_variable();

  void wait(mutex& _mutex);
  void wait(scope_lock<mutex>& _scope_lock);

  template<typename P>
  void wait(mutex& _mutex, P&& _predicate);

  template<typename P>
  void wait(scope_lock<mutex>& _scope_lock, P&& _predicate);

  void signal();
  void broadcast();

private:
  // Fixed-capacity storage for any OS condition variable type, adjust if necessary.
  alignas(16) rx_byte m_cond[64];
};

inline void condition_variable::wait(scope_lock<mutex>& _scope_lock) {
  wait(_scope_lock.m_lock);
}

template<typename P>
inline void condition_variable::wait(mutex& _mutex, P&& _predicate) {
  while (!_predicate()) {
    wait(_mutex);
  }
}

template<typename P>
inline void condition_variable::wait(scope_lock<mutex>& _scope_lock, P&& _predicate) {
  while (!_predicate()) {
    wait(_scope_lock);
  }
}

} // namespace rx::concurrency

#endif // RX_CORE_CONCURRENCY_CONDITION_VARIABLE_H
