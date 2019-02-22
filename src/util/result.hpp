#pragma once

#include <functional>
#include <memory>
#include <string>

namespace nova::renderer {
    struct nova_error {
        std::string message = "";

        std::unique_ptr<nova_error> cause;

        nova_error() = default;

        explicit nova_error(std::string&& message);

        nova_error(std::string&& message, nova_error&& cause);
    };

    template <typename ValueType>
    struct result {
        union {
            ValueType value;
            nova_error error;
        };

        bool has_value = false;

        explicit result(ValueType&& value) : value(value), has_value(true) {}

        explicit result(nova_error&& error) : error(std::forward<nova_error>(error)) {}

        result(const result<ValueType>& other) = delete;
        result<ValueType>& operator=(const result<ValueType>& other) = delete;

        result(result<ValueType>&& old) noexcept {
            if(old.has_value) {
                value = old.value;
                old.value = {};

                has_value = true;
            }
            else {
                error = old.error;
                old.error = {};
            }
        };

        result<ValueType>& operator=(result<ValueType>&& old) noexcept {
            if(old.has_value) {
                value = old.value;
                old.value = {};

                has_value = true;
            }
            else {
                error = old.error;
                old.error = {};
            }

            return *this;
        };

        ~result() {
            if(has_value) {
                value.~ValueType();
            }
            else {
                error.~nova_error();
            }
        }

        template <typename FuncType>
        auto map(FuncType&& func) -> result<decltype(func(value))> {
            using RetVal = decltype(func(value));

            if(has_value) {
                return result<RetVal>(std::move(func(value)));
            }
            else {
                return result<RetVal>(std::move(error));
            }
        }
    };
} // namespace nova::renderer