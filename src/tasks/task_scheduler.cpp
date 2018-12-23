/*!
 * \author ddubois 
 * \date 15-Dec-18.
 */

#include <utility>
#include "task_scheduler.hpp"

namespace nova::ttl {
    task_scheduler::per_thread_data::per_thread_data() : task_queue(new wait_free_queue<std::function<void()>>), things_in_queue_mutex(new std::mutex),
        things_in_queue_cv(new std::condition_variable), is_sleeping(new std::atomic<bool>(false)) {}

	task_scheduler::task_scheduler(const uint32_t num_threads, const empty_queue_behavior behavior) : num_threads(num_threads), should_shutdown(new std::atomic<bool>(false)), initialized_mutex(new std::mutex), initialized_cv(new std::condition_variable) {
		threads.reserve(num_threads);
		thread_local_data.resize(num_threads);

        for(uint32_t i = 0; i < num_threads; i++) {
            per_thread_data data;
            data.task_queue = std::make_unique<wait_free_queue<std::function<void()>>>();
            data.is_sleeping = std::make_unique<std::atomic<bool>>();
            data.last_successful_steal = 0;
            data.things_in_queue_cv = std::make_unique<std::condition_variable>();
            data.things_in_queue_mutex = std::make_unique<std::mutex>();
            thread_local_data.push_back(std::move(data));

			threads.emplace_back(thread_func, this);
        }

		{
			std::lock_guard l(*initialized_mutex);
			initialized = true;
			initialized_cv->notify_all();
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
    	size_t thread_idx = 0;
    	if(behavior_of_task_queue_search == task_queue_search_behavior::NEXT) {
			if(last_task_queue_index >= thread_local_data.size()) {
				last_task_queue_index = 0;
			}

			thread_idx = ++last_task_queue_index;
    	} else if(behavior_of_task_queue_search == task_queue_search_behavior::MOST_EMPTY) {
    		size_t lowest_size = std::numeric_limits<size_t>::max();
    		for(size_t i = 0; i < thread_local_data.size(); i++) {
    			size_t size = thread_local_data[i].task_queue->size();
    			if(size < lowest_size) {
    				thread_idx = i;
    				lowest_size = size;
    			}
    		}
    	}

		thread_local_data[thread_idx].task_queue->push(std::move(task));

		if(behavior_of_empty_queues == empty_queue_behavior::SLEEP) {
			// Find a thread that is sleeping and wake it
			for(uint32_t i = 0; i < num_threads; ++i) {
				std::unique_lock<std::mutex> lock(*thread_local_data[i].things_in_queue_mutex);
				if(thread_local_data[i].is_sleeping) {
					thread_local_data[i].is_sleeping->store(false);
					thread_local_data[i].things_in_queue_cv->notify_one();

					break;
				}
			}
		}
	}

	void task_scheduler::add_task_proxy(std::function<void()> task) {
		add_task(std::move(task));
	}

    bool task_scheduler::get_next_task(std::function<void()> *task) {
        const std::size_t current_thread_index = get_current_thread_idx();
		per_thread_data &tls = thread_local_data[current_thread_index];

		// Try to pop from our own queue
		if(tls.task_queue->pop(task)) {
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
			if(other_tls.task_queue->steal(task)) {
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
	void thread_func(task_scheduler *pool) {
		{
			std::unique_lock l(*pool->initialized_mutex);
			pool->initialized_cv->wait(l, [=] { return pool->initialized; });
		}

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
					std::unique_lock<std::mutex> lock(*tls.things_in_queue_mutex);
					tls.is_sleeping->store(true);

					tls.things_in_queue_cv->wait(lock);

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
