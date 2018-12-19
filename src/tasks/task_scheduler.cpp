/*!
 * \author ddubois 
 * \date 15-Dec-18.
 */

#include "task_scheduler.hpp"

namespace nova::ttl {
	task_scheduler::task_scheduler(const uint32_t num_threads, const empty_queue_behavior behavior) : num_threads(num_threads), should_shutdown(new std::atomic<bool>(false)) {
		threads.reserve(num_threads);

        for(uint32_t i = 0; i < num_threads; i++) {
			threads.emplace_back(thread_func, this);
        }
	}

    task_scheduler::~task_scheduler() {
		should_shutdown->store(true);

        for(auto& thread : threads) {
			thread.join();
        }
	}

    std::size_t task_scheduler::get_current_thread_idx() {
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

    uint32_t task_scheduler::get_num_threads() const {
		return num_threads;
	}

    void task_scheduler::add_task(std::function<void()> task) {
		static thread_local std::size_t external_index = 0;

		const std::size_t thread_idx = get_current_thread_idx();
		thread_local_data[get_current_thread_idx()].task_queue.push(std::move(task));

		if(behavior_of_empty_queues  == empty_queue_behavior::SLEEP) {
			// Find a thread that is sleeping and wake it
			for(uint32_t i = 0; i < num_threads; ++i) {
				std::unique_lock<std::mutex> lock(thread_local_data[i].things_in_queue_mutex);
				if(thread_local_data[i].is_sleeping) {
					thread_local_data[i].is_sleeping = false;
					thread_local_data[i].things_in_queue_cv.notify_one();

					break;
				}
			}
		}
	}

    bool task_scheduler::get_next_task(std::function<void()>* task) {
        const std::size_t current_thread_index = get_current_thread_idx();
		per_thread_data &tls = thread_local_data[current_thread_index];

		// Try to pop from our own queue
		if(tls.task_queue.pop(task)) {
			return true;
		}

		// Ours is empty, try to steal from the others'
		const std::size_t thread_index = tls.last_successful_steal;
		for(std::size_t i = 0; i < num_threads; ++i) {
			const std::size_t thread_index_to_steal_from = (thread_index + i) % num_threads;
			if(thread_index_to_steal_from == current_thread_index) {
				continue;
			}

			per_thread_data &other_tls = thread_local_data[thread_index_to_steal_from];
			if(other_tls.task_queue.steal(task)) {
				tls.last_successful_steal = thread_index_to_steal_from;
				return true;
			}
		}

		return false;
	}

    /*!
	 * \brief Function for each thread in the thread pool. We check if there's any tasks to execute. If so they get
	 * executed, if not we check again
	 */
	void thread_func(task_scheduler* pool) {
	    const std::size_t thread_idx = pool->get_current_thread_idx();
		task_scheduler::per_thread_data& tls = pool->thread_local_data[thread_idx];

		while(!pool->should_shutdown->load()) {
			// Get a new task from the queue, and execute it
			std::function<void()> next_task;
		    const bool success = pool->get_next_task(&next_task);
			const empty_queue_behavior behavior = pool->behavior_of_empty_queues;

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
					tls.is_sleeping = true;

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
