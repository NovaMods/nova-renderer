#include "rx/core/concurrency/spin_lock.h" // spin_lock
#include "rx/core/concurrency/yield.h" // yield

// ThreadSanitizer annotations
#if defined(RX_TSAN)
extern "C"
{
  void __tsan_acquire(void*);
  void __tsan_release(void*);
};
# define tsan_acquire(x) __tsan_acquire(x)
# define tsan_release(x) __tsan_release(x)
#else
# define tsan_acquire(x)
# define tsan_release(x)
#endif

namespace rx::concurrency {

void spin_lock::lock() {
  tsan_acquire(&m_lock);

  // fast path, always succeeds within a single thread
  if (!m_lock.test_and_set(memory_order::k_acquire)) {
    return;
  }

  // fixed busy loop
  int count{100};
  while (count--) {
    if (!m_lock.test_and_set(memory_order::k_acquire)) {
      return;
    }
  }

  // blocking loop
  while (m_lock.test_and_set(memory_order::k_acquire)) {
    yield();
  }
}

void spin_lock::unlock() {
  m_lock.clear(memory_order::k_release);
  tsan_release(&m_lock);
}

} // namespace rx::concurrency
