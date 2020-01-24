#ifndef RX_CORE_CONCURRENCY_CONDITION_VARIABLE_H
#define RX_CORE_CONCURRENCY_CONDITION_VARIABLE_H
#include "rx/core/config.h" // RX_PLATFORM_*

#if defined(RX_PLATFORM_POSIX)
#include <pthread.h> // pthread_cond_t
#elif defined(RX_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define VC_EXTRALEAN
#include <windows.h> // CONDITION_VARIABLE
#undef interface
#else
#error "missing condition variable implementation"
#endif

#include "rx/core/assert.h" // RX_ASSERT

#include "rx/core/concurrency/scope_lock.h" // spin_lock
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
#if defined(RX_PLATFORM_POSIX)
  pthread_cond_t m_cond;
#elif defined(RX_PLATFORM_WINDOWS)
  CONDITION_VARIABLE m_cond;
#endif
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
