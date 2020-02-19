#ifndef RX_CORE_CONCURRENCY_MUTEX_H
#define RX_CORE_CONCURRENCY_MUTEX_H
#include "rx/core/types.h" // rx_byte

namespace rx::concurrency {

struct mutex {
  mutex();
  ~mutex();

  void lock();
  void unlock();

private:
  friend struct condition_variable;

  // Fixed-capacity storage for any OS mutex type, adjust if necessary.
  alignas(16) rx_byte m_mutex[64];
};

} // namespace rx::concurrency

#endif // RX_CORE_CONCURRENCY_MUTEX_H
