#ifndef RX_CORE_CONCURRENCY_THREAD_POOL_H
#define RX_CORE_CONCURRENCY_THREAD_POOL_H
#include "rx/core/intrusive_list.h"
#include "rx/core/function.h"
#include "rx/core/dynamic_pool.h"

#include "rx/core/concurrency/thread.h"
#include "rx/core/concurrency/mutex.h"
#include "rx/core/concurrency/condition_variable.h"

#include "rx/core/hints/empty_bases.h"

namespace rx::concurrency {

struct RX_HINT_EMPTY_BASES thread_pool
  : concepts::no_copy
  , concepts::no_move
{
  thread_pool(memory::allocator* _allocator, rx_size _threads, rx_size _static_pool_size);
  thread_pool(rx_size _threads, rx_size _job_pool_size);
  ~thread_pool();

  // insert |_task| into the thread pool to be executed, the integer passed
  // to |_task| is the thread id of the calling thread in the pool
  void add(function<void(int)>&& task_);

  memory::allocator* allocator() const;

  static thread_pool& instance();

private:
  memory::allocator* m_allocator;

  mutex m_mutex;
  condition_variable m_task_cond;
  condition_variable m_ready_cond;

  intrusive_list m_queue;    // protected by |m_mutex|
  vector<thread> m_threads;  // protected by |m_mutex|
  dynamic_pool m_job_memory; // protected by |m_mutex|
  bool m_stop;               // protected by |m_mutex|

  static RX_GLOBAL<thread_pool> s_thread_pool;
};

inline thread_pool::thread_pool(rx_size _threads, rx_size _static_pool_size)
  : thread_pool{&memory::g_system_allocator, _threads, _static_pool_size}
{
}

inline memory::allocator* thread_pool::allocator() const {
  return m_allocator;
}

inline thread_pool& thread_pool::instance() {
  return *s_thread_pool;
}

} // namespace rx::concurrency

#endif // RX_CORE_CONCURRENCY_THREAD_POOL_H
