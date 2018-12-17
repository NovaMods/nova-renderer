/*!
 * \author ddubois 
 * \date 15-Dec-18.
 */

#include "thread_pool.hpp"

namespace nova::ttl {
	thread_pool::thread_pool(uint32_t num_threads) : num_threads(num_threads), should_shutdown(false) {
		threads.reserve(num_threads);

        for(uint32_t i = 0; i < num_threads; i++) {
			threads.emplace_back(thread_func, this);
        }
	}

    std::size_t thread_pool::get_current_thread_idx() {
        const std::thread::id thread_id = std::this_thread::get_id();
		for(std::size_t i = 0; i < num_threads; ++i) {
			if(threads[i].get_id() == thread_id) {
				return i;
			}
		}

        // If none of the threads in the pool are the thread we were called from, the user is doing something 
	    // unsupported so they must be punished
		throw called_from_external_thread();
	}

    /*!
	 * \brief Function for each thread in the thread pool. We check if there's any tasks to execute. If so they get
	 * executed, if not we check again
	 */
	void thread_func(thread_pool* pool) {
	    const std::size_t thread_idx = pool->get_current_thread_idx();
		thread_pool::per_thread_data& tls = pool->thread_local_data[thread_idx];

		while(!pool->should_shutdown.load(std::memory_order_acquire)) {
			// Get a new task from the queue, and execute it
			std::function<void()> next_task;
		    const bool success = pool->get_next_task(&next_task);
		    const empty_queue_behavior behavior = pool->behavior_of_empty_queues.load(std::memory_order::memory_order_relaxed);

			if(success) {
				next_task();

			} else {
				// We failed to find a Task from any of the queues
				// What we do now depends on behavior_of_empty_queues, which we loaded above
				switch(behavior) {
				case empty_queue_behavior::YIELD:
					std::this_thread::yield();
					break;

				case empty_queue_behavior::SLEEP:
				{
					std::unique_lock<std::mutex> lock(tls.things_in_queue_mutex);

					tls.things_in_queue_cv.wait(lock);

					break;
				}
				
				case empty_queue_behavior::SPIN:
				default:
					// Just fall through and continue the next loop
					break;
				}
			}
		}
	}

}
