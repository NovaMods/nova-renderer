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
#include "../util/logger.hpp"
#include "condition_counter.hpp"
#include "task_graph.hpp"

#ifdef __linux__
#include "../util/linux_utils.hpp"
#endif

namespace nova::ttl {
	NOVA_EXCEPTION(called_from_external_thread);

	/*!
	 * \brief Represents a chain of tasks that must be executed in a certain order
	 *
	 * This class is awesome because it doesn't submit a task until all the tasks it depends on are finished. This
	 * ensures that we _never_ run into thread exhaustion issues
	 *
	 * This class is not thread-safe
	 */
	template <typename InputType, typename OutputType>
	struct task_node {
		task_node(std::function<OutputType(InputType)> func) : operation(func) {};

		template <typename SuccessHandlerOutputType>
		task_node& on_success(task_node<OutputType, SuccessHandlerOutputType>&& success_func) {
			success_handler = [success_func = std::move(success_func)](const OutputType& output) { success_func.operation(output); };

			return *this;
		}

		template <typename ErrorHandlerOutputType>
		task_node& on_error(task_node<ErrorHandlerOutputType, const std::exception&>&& error_func) {
			error_handler = [error_func = std::move(error_func)](const std::exception& exception) { error_func(exception); };

			return *this;
		}

		std::future<OutputType> get_future() { return promise->get_future(); }

		std::function<OutputType(InputType)> operation;
		std::function<void(const OutputType&)> success_handler = [](const OutputType& output) {};
		std::function<void(const std::exception&)> error_handler = [](const std::exception& e) {
			NOVA_LOG(ERROR) << "Exception during task execution: " << e.what();

#ifdef __linux__
			nova_backtrace();
#endif
			std::rethrow_exception(std::current_exception());
		};

	private:
		std::shared_ptr<std::promise<OutputType>> promise;
	};

	template<typename InputType>
	struct task_node <InputType, void> {
		task_node(std::function<void(InputType)> func) : operation(func) {};

		template <typename SuccessHandlerOutputType>
		task_node<InputType, void>& on_success(task_node<void, SuccessHandlerOutputType>&& success_func) {
			success_handler = [success_func = std::move(success_func)]() { success_func(); };

			return *this;
		}

		template <typename ErrorHandlerOutputType>
		task_node& on_error(task_node<ErrorHandlerOutputType, const std::exception&>&& error_func) {
			error_handler = [error_func = std::move(error_func)](const std::exception& exception) { error_func(exception); };

			return *this;
		}

		std::function<void(InputType)> operation;
		std::function<void()> success_handler = [] {};
		std::function<void(const std::exception&)> error_handler = [](const std::exception& e) {
			NOVA_LOG(ERROR) << "Exception during task execution: " << e.what();

#ifdef __linux__
			nova_backtrace();
#endif
			std::rethrow_exception(std::current_exception());
		};
	};

    template<typename... Outputs>
	task_node(Outputs... args) -> task_node<void, std::tuple<Outputs...>>;

	template<typename... Outputs>
    struct task_node<void, std::tuple<Outputs...>> {
	    task_node(std::function<Outputs()> ops);

		template <typename SuccessHandlerOutputType>
		task_node<void, std::tuple<Outputs...>>& on_success(task_node < std::tuple<Outputs...>, SuccessHandlerOutputType);
    };

    class task_scheduler;

	using ArgumentExtractorType = std::function<void(task_scheduler *)>;

	typedef void(*TaskFunction)(task_scheduler *task_scheduler, void *arg);

	struct task {
		TaskFunction function;
		void *arg_data;
	};

    inline void argument_extractor(task_scheduler *scheduler, void *arg) {
		auto *func = static_cast<ArgumentExtractorType *>(arg);

		(*func)(scheduler);

		delete func;
	}

    /*!
     * \brief What a thread should do when there's no new tasks
     */
    enum class empty_queue_behavior {
        /*!
         * \brief Keep polling the task queue until there's a task
         */
        SPIN,

        /*!
         * \brief Yield to the OS after each poll
         */
        YIELD,

        /*!
         * \brief Sleep until tasks are available
         */
        SLEEP
    };

    /*!
     * \brief How a task queue is searched for adding a new task
     */
    enum class task_queue_search_behavior {
        /*!
         * \brief Just choose the next queue from the previously filled one and start from the beginning if none is left
         */
        NEXT,

        /*!
         * \brief Search for the most empty queue and add the task to it
         */
        MOST_EMPTY
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
			std::unique_ptr<wait_free_queue<std::function<void()>>> task_queue;
            /*!
             * \brief The index of the queue we last stole from
             */
			std::size_t last_successful_steal = 0;

			std::unique_ptr<std::mutex> things_in_queue_mutex;
			std::unique_ptr<std::condition_variable> things_in_queue_cv;
			std::unique_ptr<std::atomic<bool>> is_sleeping;

			per_thread_data();

			per_thread_data(per_thread_data&& other) noexcept = default;
			per_thread_data& operator=(per_thread_data&& other) noexcept = default;

			per_thread_data(const per_thread_data& other) = delete;
			per_thread_data& operator=(const per_thread_data& other) = delete;
        };

        /*!
         * \brief Initializes this thread pool with `num_threads` threads
         * 
         * \param num_threads The number of threads for this thread pool
         * \param behavior The behavior of empty task queues. See \enum empty_queue_behavior for more info
         */
        task_scheduler(const uint32_t num_threads, const empty_queue_behavior behavior);
        
		task_scheduler(task_scheduler&& other) noexcept = default;
		task_scheduler& operator=(task_scheduler&& other) noexcept = default;

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

			auto task = std::make_shared<std::packaged_task<RetVal()>>(std::bind(std::forward<F>(function), this, std::forward<Args>(args)...));
			std::future<RetVal> future = task->get_future();

            add_task_proxy([task] {
                try {
                    task.get()->operator()();
                } catch (...) {
                    // TODO: Better way of giving the user a chance to handle this, see https://en.cppreference.com/w/cpp/error/current_exception
                    NOVA_LOG(FATAL) << "Task failed executing!";
#ifdef __linux__
                    nova_backtrace();
#endif
                    std::rethrow_exception(std::current_exception());
                }
            });

			return future;
		}

		/*!
		 * \brief Similar to `add_task`, but this one adds a graph of tasks that 
		 */
		template<typename TaskInputType, typename TaskOutputType>
		void add_task_graph(task_node<TaskOutputType, TaskInputType>&& task, TaskInputType&& input) {
			add_task([task = std::forward(task), input = std::forward(input)] {
			    try {
					TaskOutputType output = task.operation(input);
					add_task(task.success_handler, output);

			    } catch(const std::exception& e) {
					add_task(task.error_handler, e);
			    }
			});
		}

		/*!
		 * \brief Similar to `add_task`, but this one adds a graph of tasks that
		 */
		template<typename TaskOutputType>
		void add_task_graph(task_node<void, TaskOutputType>&& task) {
			add_task([this, task = std::move(task)]{
				try {
					TaskOutputType output = task.operation();
					add_task_proxy([=] { task.success_handler(output); });

				} catch(const std::exception& e) {
					add_task_proxy([=] { task.error_handler(e); });
				}
				});
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

			auto task = std::make_shared<std::packaged_task<RetVal()>>(std::bind(function, this, std::forward<Args>(args)...));
			std::future<RetVal> future = task->get_future();

			counter->add(1);
            add_task_proxy([task, counter] {
                try {
                    task.get()->operator()();
                    counter->sub(1);
                } catch (...) {
                    // TODO: Better way of giving the user a chance to handle this, see https://en.cppreference.com/w/cpp/error/current_exception
                    NOVA_LOG(FATAL) << "Task failed executing!";
#ifdef __linux__
                    nova_backtrace();
#endif
                }
            });

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

		std::unique_ptr<std::atomic<bool>> should_shutdown;
    
		empty_queue_behavior behavior_of_empty_queues = empty_queue_behavior::YIELD;
		task_queue_search_behavior behavior_of_task_queue_search = task_queue_search_behavior::NEXT;
        bool initialized = false;
		std::unique_ptr<std::mutex> initialized_mutex;
        std::unique_ptr<std::condition_variable> initialized_cv;

        uint32_t last_task_queue_index = 0;

        /*!
	     * \brief Adds a task to the internal queue.
	     *
	     * \param task       The task to queue
	     */
		void add_task(std::function<void()> task);
		/*!
		 * \brief Proxy to add_task because the compiler cannot be sure which add_task
		 * 		  to use for lambdas
		 * \param task The task to queue
		 */
		void add_task_proxy(std::function<void()> task);

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
