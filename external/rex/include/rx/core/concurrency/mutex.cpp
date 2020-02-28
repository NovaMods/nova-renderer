#include "rx/core/concurrency/mutex.h"
#include "rx/core/config.h" // RX_PLATFORM_{POSIX,WINDOWS}
#include "rx/core/assert.h"

#if defined(RX_PLATFORM_POSIX)
#include <pthread.h> // pthread_mutex_t, pthread_mutex_{init,destroy,lock,unlock}
#elif defined(RX_PLATFORM_WINDOWS)
#define _WIN32_LEAN_AND_MEAN
#include <windows.h> // CRITICAL_SECTION, {Initialize,Delete,Enter,Leave}CriticalSection
#else
#error "missing mutex implementation"
#endif

namespace rx::concurrency {

mutex::mutex() {
#if defined(RX_PLATFORM_POSIX)
  auto handle = reinterpret_cast<pthread_mutex_t*>(m_mutex);
  if (pthread_mutex_init(handle, nullptr) != 0) {
    RX_ASSERT(false, "initialization failed");
  }
#elif defined(RX_PLATFORM_WINDOWS)
  auto handle = reinterpret_cast<CRITICAL_SECTION*>(m_mutex);
  InitializeCriticalSection(handle);
#endif
}

mutex::~mutex() {
#if defined(RX_PLATFORM_POSIX)
  auto handle = reinterpret_cast<pthread_mutex_t*>(m_mutex);
  if (pthread_mutex_destroy(handle) != 0) {
    RX_ASSERT(false, "destroy failed");
  }
#elif defined(RX_PLATFORM_WINDOWS)
  auto handle = reinterpret_cast<CRITICAL_SECTION*>(m_mutex);
  DeleteCriticalSection(handle);
#endif
}

void mutex::lock() {
#if defined(RX_PLATFORM_POSIX)
  auto handle = reinterpret_cast<pthread_mutex_t*>(m_mutex);
  if (pthread_mutex_lock(handle) != 0) {
    RX_ASSERT(false, "lock failed");
  }
#elif defined(RX_PLATFORM_WINDOWS)
  auto handle = reinterpret_cast<CRITICAL_SECTION*>(m_mutex);
  EnterCriticalSection(handle);
#endif
}

void mutex::unlock() {
#if defined(RX_PLATFORM_POSIX)
  auto handle = reinterpret_cast<pthread_mutex_t*>(m_mutex);
  if (pthread_mutex_unlock(handle) != 0) {
    RX_ASSERT(false, "unlock failed");
  }
#elif defined(RX_PLATFORM_WINDOWS)
  auto handle = reinterpret_cast<CRITICAL_SECTION*>(m_mutex);
  LeaveCriticalSection(handle);
#endif
}

} // namespace rx::concurrency
