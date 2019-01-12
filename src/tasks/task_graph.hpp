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

		std::function<OutputType(InputType)> operation;
		std::function<void(const OutputType&)> success_handler = [](const OutputType& output) {};
		std::function<void(const std::exception&)> error_handler = [](const std::exception& e) {
			NOVA_LOG(ERROR) << "Exception during task execution: " << e.what();

#ifdef __linux__
			nova_backtrace();
#endif
			std::rethrow_exception(std::current_exception());
		};
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
		std::function<void(void)> success_handler = [] {};
		std::function<void(const std::exception&)> error_handler = [](const std::exception& e) {
			NOVA_LOG(ERROR) << "Exception during task execution: " << e.what();

#ifdef __linux__
			nova_backtrace();
#endif
			std::rethrow_exception(std::current_exception());
		};
	};
}
