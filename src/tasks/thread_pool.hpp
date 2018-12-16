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

namespace nova {
	class thread_pool;

	using argument_extractor_type = std::function<void(thread_pool*)>;

	typedef void(*task_function)(thread_pool *task_scheduler, void *arg);

	struct task {
		task_function function;
		void *arg_data;
	};

    inline void argument_extractor(thread_pool* scheduler, void* arg) {
		auto* func = static_cast<argument_extractor_type*>(arg);

		(*func)(scheduler);

		delete func;
	}

    /*!
     * \brief A thread pool for Nova!
     */
    class thread_pool {
    public:
		thread_pool(uint32_t num_threads);

		thread_pool(const thread_pool& other) = delete;

		thread_pool(thread_pool&& other);

		~thread_pool();

		thread_pool& operator=(const thread_pool& other) = delete;
		thread_pool& operator=(thread_pool&& other);

		/**
	     * Adds a task to the internal queue. Allocates internally
	     *
	     * \tparam F       Function type.
	     * \tparam Args    Arguments to the function. Copied if lvalue. Moved if rvalue. Use std::ref/std::cref for references.
	     * 
	     * \param counter  An atomic counter corresponding to this task. Gets incremented when this method is called, and 
	     * decremented when the task finishes
	     * \param function Function to invoke
	     * \param args     Arguments to the function. Copied if lvalue. Moved if rvalue. Use std::ref/std::cref for references.
	     */
		template<class F, class... Args>
		void add_task(std::atomic<uint32_t>* counter, F&& function, Args&&... args) {
			auto bound_func = std::bind(function, std::placeholders::_1, std::forward<Args>(args)...);
			auto* alloced_function = new argument_extractor_type(bound_func);

			add_task(task{ argument_extractor, alloced_function }, counter);
		}

		/**
		 * Adds a task to the internal queue. Allocates internally
		 *
		 * \tparam F       Function type.
		 * \tparam Args    Arguments to the function. Copied if lvalue. Moved if rvalue. Use std::ref/std::cref for references.
		 *
		 * decremented when the task finishes
		 * \param function Function to invoke
		 * \param args     Arguments to the function. Copied if lvalue. Moved if rvalue. Use std::ref/std::cref for references.
		 */
		template<class F, class... Args>
		auto add_typed_task(F&& function, Args&&... args)
			-> std::future<decltype(function(std::declval<thread_pool*>(), std::forward<Args>(args)...))> {
			using RetVal = decltype(function(std::declval<thread_pool*>(), std::forward<Args>(args)...));

			std::promise<RetVal>* promise = detail::create_promise(this, nullptr, 0, function, std::forward<Args>(args)...);

			std::future<RetVal> future = promise->get_future();

			add_task(task{ detail::TypeSafeTask<RetVal>, promise }, promise->counter());

			return future;
		}

    private:
		/**
	     * Adds a task to the internal queue.
	     *
	     * \param task       The task to queue
	     * \param counter    An atomic counter corresponding to this task. Gets incremented when this method is called, and 
	     * decremented when the task finishes
	     */
		void add_task(task task, std::atomic<uint32_t> *counter = nullptr);
    };
}


#endif //NOVA_RENDERER_THREAD_POOL_HPP
