#ifndef RX_CORE_CONCURRENCY_MUTEX_H
#define RX_CORE_CONCURRENCY_MUTEX_H
#include "rx/core/config.h" // RX_PLATFORM_*

#if defined(RX_PLATFORM_POSIX)
#include <pthread.h> // pthread_mutex_t
#elif defined(RX_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define VC_EXTRALEAN
#include <windows.h> // CRITICAL_SECTION
#undef interface
#else
#error "missing mutex implementation"
#endif

namespace rx::concurrency {

struct mutex {
  mutex();
  ~mutex();

  void lock();
  void unlock();

private:
  friend struct condition_variable;

#if defined(RX_PLATFORM_POSIX)
  pthread_mutex_t m_mutex;
#elif defined(RX_PLATFORM_WINDOWS)
  CRITICAL_SECTION m_mutex;
#else
#error "missing mutex implementation"
#endif
};

} // namespace rx::concurrency

#endif // RX_CORE_CONCURRENCY_MUTEX_H
