#include "rx/core/concurrency/thread_pool.h"
#include "rx/core/concurrency/wait_group.h"

#include "rx/core/log.h"
#include "rx/core/time.h"

namespace rx::concurrency {

RX_LOG("thread_pool", logger);
RX_GLOBAL<thread_pool> thread_pool::s_thread_pool{"system", "thread_pool", 4_z};

thread_pool::thread_pool(memory::allocator* _allocator, rx_size _threads)
  : m_allocator{_allocator}
  , m_queue{m_allocator}
  , m_threads{m_allocator}
  , m_stop{false}
{
  logger(log::level::k_info, "starting pool with %zu threads", _threads);
  const auto beg{query_performance_counter_ticks()};
  m_threads.reserve(_threads);

  wait_group group{_threads};
  for (rx_size i{0}; i < _threads; i++) {
    m_threads.emplace_back("thread pool", [this, &group](int _thread_id) {
      logger(log::level::k_info, "starting thread %d", _thread_id);

      group.signal();

      for (;;) {
        function<void(int)> task;
        {
          scope_lock lock{m_mutex};
          m_task_cond.wait(lock, [this] { return m_stop || !m_queue.is_empty(); });
          if (m_stop && m_queue.is_empty()) {
            logger(log::level::k_info, "stopping thread %d", _thread_id);
            return;
          }
          task = m_queue.pop();
        }

        logger(log::level::k_verbose, "starting task on thread %d", _thread_id);
        const auto beg{query_performance_counter_ticks()};
        task(_thread_id);
        const auto end{query_performance_counter_ticks()};
        const auto time{static_cast<rx_f64>(((end - beg) * 1000.0) / static_cast<rx_f64>(query_performance_counter_frequency()))};
        logger(log::level::k_verbose, "finished task on thread %d (took %.2f ms)", _thread_id, time);
      }
    });
  }

  // wait for all threads to start
  group.wait();

  const auto end{query_performance_counter_ticks()};
  const auto time{static_cast<rx_f64>(((end - beg) * 1000.0) / static_cast<rx_f64>(query_performance_counter_frequency()))};
  logger(log::level::k_info, "started pool with %zu threads (took %.2f ms)", _threads, time);
}

thread_pool::~thread_pool() {
  const auto beg{query_performance_counter_ticks()};
  {
    scope_lock lock{m_mutex};
    m_stop = true;
  }
  m_task_cond.broadcast();

  m_threads.each_fwd([](thread &_thread) {
    _thread.join();
  });

  const auto end{query_performance_counter_ticks()};
  const auto time{static_cast<rx_f64>(((end - beg) * 1000.0) / static_cast<rx_f64>(query_performance_counter_frequency()))};
  logger(log::level::k_verbose, "stopped pool with %zu threads (took %.2f ms)", m_threads.size(), time);
}

void thread_pool::add(function<void(int)>&& task_) {
  {
    scope_lock lock(m_mutex);
    m_queue.push(utility::move(task_));
  }
  m_task_cond.signal();
}

} // namespace rx::concurrency
