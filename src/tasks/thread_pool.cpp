/*!
 * \author ddubois 
 * \date 15-Dec-18.
 */

#include "thread_pool.hpp"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#endif

namespace nova::ttl {
	thread_pool::thread_pool(uint32_t num_threads) : should_shutdown(false) {}

    std::size_t thread_pool::get_current_thread_idx() {
#if defined(_WIN32)
		DWORD threadId = GetCurrentThreadId();
		for(std::size_t i = 0; i < num_threads; ++i) {
			if(m_threads[i].Id == threadId) {
				return i;
			}
		}
#elif defined(FTL_POSIX_THREADS)
		pthread_t currentThread = pthread_self();
		for(std::size_t i = 0; i < m_numThreads; ++i) {
			if(pthread_equal(currentThread, m_threads[i])) {
				return i;
			}
		}
#endif

		throw called_from_external_thread();
	}

    /*!
	 * \brief Function for each thread in the thread pool. We check if there's any tasks to execute. If so they get
	 * executed, if not we check again
	 */
	void thread_func(thread_pool* pool) {
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
					std::unique_lock<std::mutex> lock(tls.FailedQueuePopLock);

					// Check if we have a ready fiber
					// Lock
					while(tls.ReadFibersLock.test_and_set(std::memory_order_acquire)) {
						// Spin
						FTL_PAUSE();
					}
					// Prevent sleepy-time if we have ready fibers
					if(tls.ReadyFibers.empty()) {
						++tls.FailedQueuePopAttempts;
					}

					// Unlock
					tls.ReadFibersLock.clear(std::memory_order_release);

					// Go to sleep if we've failed to find a task kFailedPopAttemptsHeuristic times
					while(tls.FailedQueuePopAttempts >= kFailedPopAttemptsHeuristic) {
						tls.FailedQueuePopCV.wait(lock);
					}

					break;
				}
				case empty_queue_behavior::SPIN:
				default:
					// Just fall through and continue the next loop
					break;
				}
			}
		}


		// Start the quit sequence

		// Switch to the thread fibers
		ThreadLocalStorage &tls = taskScheduler->m_tls[taskScheduler->GetCurrentThreadIndex()];
		taskScheduler->m_fibers[tls.CurrentFiberIndex].SwitchToFiber(&tls.ThreadFiber);


		// We should never get here
		printf("Error: FiberStart should never return");
	}

}
