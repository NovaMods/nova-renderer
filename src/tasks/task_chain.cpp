#include "task_chain.hpp"

#include "task_scheduler.hpp"

namespace nova::ttl {
    task_chain::task_chain(task_scheduler& scheduler) : scheduler(scheduler), task_graph(1) {}

    void task_chain::operator()(task_scheduler& scheduler) {
		for(std::vector<std::function<void*(void*)>>& tasks : task_graph) {
			std::vector<std::future<void*>> output_futures;
			output_futures.reserve(tasks.size());

			for(std::function<void*(void*)>& func : tasks) {
				std::future<void*> output_future = scheduler.add_task([&, func = std::move(func)](task_scheduler* scheduler) {
					return func(&last_stage_output);
				});

				output_futures.push_back(output_future);
			}

			last_stage_output.clear();
			last_stage_output.reserve(output_futures.size());

            for(std::future<void*>& output_future : output_futures) {
				output_future.wait();
				last_stage_output.push_back(output_future.get());
            }
		}
    }

    task_chain& task_chain::add_task_list(std::vector<std::function<void*(void*)>>& functions) {
		for(std::function<void*(void*)>& function : functions) {
			add_task(function);
		}

		return *this;
	}

    void task_chain::add_layer() {
		last_stage_output.push_back({});
    }
}
