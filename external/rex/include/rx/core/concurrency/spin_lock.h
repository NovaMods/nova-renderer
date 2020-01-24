#ifndef RX_CORE_CONCURRENCY_SPIN_LOCK_H
#define RX_CORE_CONCURRENCY_SPIN_LOCK_H
#include "rx/core/concurrency/atomic.h" // atomic_flag

namespace rx::concurrency {

struct spin_lock {
  constexpr spin_lock();
  ~spin_lock() = default;
  void lock();
  void unlock();
private:
  atomic_flag m_lock;
};

inline constexpr spin_lock::spin_lock()
  : m_lock{false}
{
}

} // namespace rx::concurrency

#endif // RX_CORE_CONCURRENCY_SPIN_LOCK_H
