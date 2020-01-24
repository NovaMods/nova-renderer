#include "rx/core/concurrency/mutex.h"
#include "rx/core/assert.h"

namespace rx::concurrency {

mutex::mutex() {
#if defined(RX_PLATFORM_POSIX)
  if (pthread_mutex_init(&m_mutex, nullptr) != 0) {
    RX_ASSERT(false, "initialization failed");
  }
#elif defined(RX_PLATFORM_WINDOWS)
  InitializeCriticalSection(&m_mutex);
#endif
}

mutex::~mutex() {
#if defined(RX_PLATFORM_POSIX)
  if (pthread_mutex_destroy(&m_mutex) != 0) {
    RX_ASSERT(false, "destroy failed");
  }
#elif defined(RX_PLATFORM_WINDOWS)
  DeleteCriticalSection(&m_mutex);
#endif
}

void mutex::lock() {
#if defined(RX_PLATFORM_POSIX)
  if (pthread_mutex_lock(&m_mutex) != 0) {
    RX_ASSERT(false, "lock failed");
  }
#elif defined(RX_PLATFORM_WINDOWS)
  EnterCriticalSection(&m_mutex);
#endif
}

void mutex::unlock() {
#if defined(RX_PLATFORM_POSIX)
  if (pthread_mutex_unlock(&m_mutex) != 0) {
    RX_ASSERT(false, "unlock failed");
  }
#elif defined(RX_PLATFORM_WINDOWS)
  LeaveCriticalSection(&m_mutex);
#endif
}

} // namespace rx::concurrency
