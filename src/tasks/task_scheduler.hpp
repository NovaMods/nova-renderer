/*!
 * \author ddubois 
 * \date 15-Dec-18.
 */

#ifndef NOVA_RENDERER_THREAD_POOL_HPP
#define NOVA_RENDERER_THREAD_POOL_HPP

#include <cstdint>
#include <atomic>
#include <functional>
#include <future>
#include "wait_free_queue.hpp"
#include "../util/utils.hpp"
#include "condition_counter.hpp"

namespace nova::ttl {
	NOVA_EXCEPTION(called_from_external_thread);

	class task_scheduler;

	using ArgumentExtractorType = std::function<void(task_scheduler*)>;

	typedef void(*TaskFunction)(task_scheduler *task_scheduler, void *arg);

	struct task {
		TaskFunction function;
		void *arg_data;
	};

    inline void argument_extractor(task_scheduler* scheduler, void* arg) {
		auto* func = static_cast<ArgumentExtractorType*>(arg);

		(*func)(scheduler);

		delete func;
	}

    /*!
     * What a thread should do when there's no new tasks
     */
    enum class empty_queue_behavior {
        /*!
         * Keep polling the task queue until there's a task
         */
        SPIN,

        /*!
         * Yield to the OS after each poll
         */
        YIELD,

        /*!
         * Sleep until tasks are available
         */
        SLEEP
    };

    /*!
     * \brief A thread pool for Nova!
     */
    class task_scheduler {
    public:
        /*!
         * \brief Data that each thread needs
         */
        struct per_thread_data {
            /*!
             * \brief A queue of all the tasks this thread needs to execute
             */
			wait_free_queue<std::function<void()>> task_queue;
            /*!
             * \brief The index of the queue we last stole from
             */
			std::size_t last_successful_steal;

			std::mutex things_in_queue_mutex;
			std::condition_variable things_in_queue_cv;
			std::atomic<bool> is_sleeping = false;
        };

        /*!
         * \brief Initializes this thread pool with `num_threads` threads
         * 
         * \param num_threads The number of threads for this thread pool
         */
        explicit task_scheduler(uint32_t num_threads);
        
		task_scheduler(task_scheduler&& other) noexcept = default;
		task_scheduler& operator=(task_scheduler&& other) = default;

		~task_scheduler();

		task_scheduler(const task_scheduler& other) = delete;
		task_scheduler& operator=(const task_scheduler& other) = delete;
        
		/*!
		 * \brief Adds a task to the internal queue. Allocates internally
		 *
		 * \tparam F       Function type.
		 * \tparam Args    Arguments to the function. Copied if lvalue. Moved if rvalue. Use std::ref/std::cref for references.
		 *
		 * \param function Function to invoke
		 * \param args     Arguments to the function. Copied if lvalue. Moved if rvalue. Use std::ref/std::cref for references.
		 * 
		 * \return A future to the data that your task will produce
		 */
		template<class F, class... Args>
		auto add_task(F&& function, Args&&... args)
			-> std::future<decltype(function(std::declval<task_scheduler*>(), std::forward<Args>(args)...))> {
			using RetVal = decltype(function(std::declval<task_scheduler*>(), std::forward<Args>(args)...));

			std::packaged_task<RetVal()> task(std::bind(function, this, std::forward<Args>(args)...));
			std::future<RetVal> future = task.get_future();

			add_task([moved_task = std::move(task)] { moved_task(); });

			return future;
		}

		/*!
		 * \brief Adds a task to the internal queue. Allocates internally
		 *
		 * \tparam F       Function type.
		 * \tparam Args    Arguments to the function. Copied if lvalue. Moved if rvalue. Use std::ref/std::cref for references.
		 *
		 * \param counter  The counter to decrement when the task has finished
		 * \param function Function to invoke
		 * \param args     Arguments to the function. Copied if lvalue. Moved if rvalue. Use std::ref/std::cref for references.
		 *
		 * \return A future to the data that your task will produce
		 */
		template<class F, class... Args>
		auto add_task(condition_counter* counter, F&& function, Args&&... args)
			-> std::future<decltype(function(std::declval<task_scheduler*>(), std::forward<Args>(args)...))> {
			using RetVal = decltype(function(std::declval<task_scheduler*>(), std::forward<Args>(args)...));

			std::packaged_task<RetVal()> task(std::bind(function, this, std::forward<Args>(args)...));
			std::future<RetVal> future = task.get_future();

			add_task([moved_task = std::move(task)]{ moved_task(); counter->sub(1); });

			return future;
		}

        /*!
         * \brief Gets the index of the current thread
         * 
         * Gets the ID of the thread this method is called from. Loops through all the threads that TTL knows about,
         * comparing their IDs to our ID. If a match is found, the index of that thread is returned. If not, an 
         * exception is thrown
         * 
         * \return The index of the calling thread
         */
		std::size_t get_current_thread_idx();

		friend void thread_func(task_scheduler* pool);

        uint32_t get_num_threads() const;
        
    private:
		uint32_t num_threads;
		std::vector<std::thread> threads;
		std::vector<per_thread_data> thread_local_data;

		std::atomic<bool> should_shutdown;
    
		std::atomic<empty_queue_behavior> behavior_of_empty_queues;

        /*!
	     * \brief Adds a task to the internal queue.
	     *
	     * \param task       The task to queue
	     */
		void add_task(std::function<void()> task);

        /*!
         * \brief Attempts to get the next task, returning success
         * 
         * \param task The memory to write the next task to
         * \return True if there was a task, false if there was not
         */
		bool get_next_task(std::function<void()> *task);
    };

	void thread_func(task_scheduler* pool);
}


#endif //NOVA_RENDERER_THREAD_POOL_HPP
