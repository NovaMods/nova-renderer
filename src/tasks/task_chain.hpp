#pragma once
#include <vector>
#include <functional>
#include <future>
#include "../util/logger.hpp"
#include "../util/utils.hpp"

#include "task_scheduler.hpp"

namespace nova::ttl {
    /*!
     * \brief Represents a chain of tasks that must be executed in a certain order
     * 
     * This class is awesome because it doesn't submit a task until all the tasks it depends on are finished. This 
     * ensures that we _never_ run into thread exhaustion issues
     * 
     * This class is not thread-safe
     */
    class task_chain {
    public:
		task_chain(task_scheduler& scheduler);

		void operator()(task_scheduler& scheduler);
        
		template<class F, class... Args>
		task_chain& add_task(F&& function, Args&&... args) {
			std::vector<std::function<void*(void*)>>& tasks = task_graph.back();

			using RetVal = decltype(function(std::placeholders::_1, std::forward<Args>(args)...));

			auto task = std::make_shared<std::packaged_task<RetVal()>>(std::bind(std::forward<F>(function), std::placeholders::_1, std::forward<Args>(args)...));

			tasks.push_back(std::bind(apply_task, std::placeholders::_1, task));

			return *this;
		}

		task_chain& add_task_list(std::vector<std::function<void*(void*)>>& functions);

		void add_layer();

    private:
		task_scheduler& scheduler;

		std::vector<std::vector<std::function<void*(void*)>>> task_graph;

		std::vector<void*> last_stage_output;

		template<typename InputType, typename OutputType>
		OutputType apply_task(std::shared_ptr<std::packaged_task<OutputType>> task, InputType input) {
			try {
				return task.get()->operator()(input);

			} catch(...) {
				// TODO: Better way of giving the user a chance to handle this, see https://en.cppreference.com/w/cpp/error/current_exception
				NOVA_LOG(FATAL) << "Task failed executing!";
#ifdef __linux__
				nova_backtrace();
#endif
				std::rethrow_exception(std::current_exception());
			}
		}
    };
}
