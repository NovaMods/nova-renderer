#pragma once

#include <functional>
#include <memory>
#include <string>

#include <fmt/format.h>

#include "utils.hpp"

namespace nova::renderer {
    struct nova_error {
        std::string message = "";

        std::unique_ptr<nova_error> cause;

        nova_error() = default;

        explicit nova_error(std::string message);

        nova_error(std::string message, nova_error cause);

        [[nodiscard]] std::string to_string() const;
    };

    inline nova_error operator""_err(const char* str, std::size_t size) { return nova_error(std::string(str)); }

    template <typename ValueType>
    struct result {
        union {
            ValueType value;
            nova_error error;
        };

        bool has_value = false;

        explicit result(ValueType&& value) : value(value), has_value(true) {}

        explicit result(const ValueType& value) : value(value), has_value(true) {}

        explicit result(nova_error error) : error(std::move(error)) {}

        result(const result<ValueType>& other) = delete;
        result<ValueType>& operator=(const result<ValueType>& other) = delete;

        result(result<ValueType>&& old) noexcept {
            if(old.has_value) {
                value = std::move(old.value);
                old.value = {};

                has_value = true;
            }
            else {
                error = std::move(old.error);
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
                return result<RetVal>(func(value));
            }
            else {
                return result<RetVal>(std::move(error));
            }
        }

        template <typename FuncType>
        auto flatMap(FuncType&& func) -> result<decltype(func(value).value)> {
            using RetVal = decltype(func(value).value);

            if(has_value) {
                return func(value);
            }
            else {
                return result<RetVal>(std::move(error));
            }
        }

        void on_error(std::function<void(const nova_error&)> error_func) const {
            if(!has_value) {
                error_func(error);
            }
        }
    };

    template <typename ValueType, typename... Args>
    result<ValueType> error_result(fmt::compile_string s, Args... args) {
        return result<ValueType>(nova_error(fmt::format(s, args)));
    }

} // namespace nova::renderer