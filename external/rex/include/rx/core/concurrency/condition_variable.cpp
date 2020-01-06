#include "rx/core/concurrency/condition_variable.h"

namespace rx::concurrency {

condition_variable::condition_variable() {
#if defined(RX_PLATFORM_POSIX)
  if (pthread_cond_init(&m_cond, nullptr) != 0) {
    RX_ASSERT(false, "failed to initialize");
  }
#elif defined(RX_PLATFORM_WINDOWS)
  InitializeConditionVariable(&m_cond);
#endif
}

condition_variable::~condition_variable() {
#if defined(RX_PLATFORM_POSIX)
  if (pthread_cond_destroy(&m_cond) != 0) {
    RX_ASSERT(false, "failed to destroy");
  }
#elif defined(RX_PLATFORM_WINDOWS)
  // NOTE: there's no function to destroy a CONDITION_VARIABLE on Windows
#endif
}

void condition_variable::wait([[maybe_unused]] mutex& _mutex) {
#if defined(RX_PLATFORM_POSIX)
  if (pthread_cond_wait(&m_cond, &_mutex.m_mutex) != 0) {
    RX_ASSERT(false, "failed to wait");
  }
#elif defined(RX_PLATFORM_WINDOWS)
  if (!SleepConditionVariableCS(&m_cond, &_mutex.m_mutex, INFINITE)) {
    RX_ASSERT(false, "failed to wait");
  }
#endif
}

void condition_variable::signal() {
#if defined(RX_PLATFORM_POSIX)
  if (pthread_cond_signal(&m_cond) != 0) {
    RX_ASSERT(false, "failed to signal");
  }
#elif defined(RX_PLATFORM_WINDOWS)
  WakeConditionVariable(&m_cond);
#endif
}

void condition_variable::broadcast() {
#if defined(RX_PLATFORM_POSIX)
  if (pthread_cond_broadcast(&m_cond) != 0) {
    RX_ASSERT(false, "failed to broadcast");
  }
#elif defined(RX_PLATFORM_WINDOWS)
  WakeAllConditionVariable(&m_cond);
#endif
}

} // namespace rx::concurrency
