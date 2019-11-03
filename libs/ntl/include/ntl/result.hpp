#pragma once

#include <functional>
#include <memory>
#include <string>

// ReSharper thinks that the include isn't used, but it's used in a macro so it needs to be here
// ReSharper disable once CppUnusedIncludeDirective
#include <fmt/format.h>

#include <nova_renderer/util/utils.hpp>

namespace ntl {
    struct NovaError {
        std::string message = "";

        std::unique_ptr<NovaError> cause;
        
        explicit NovaError(const std::string& message);

        NovaError(const std::string& message, NovaError cause);

        [[nodiscard]] std::string to_string() const;
    };

    inline NovaError operator""_err(const char* str, const std::size_t size) { return NovaError(std::string(str, size)); }

    template <typename ValueType, typename ErrorType = NovaError>
    struct Result {
        union {
            ValueType value;
            ErrorType error;
        };

        bool is_moved = false;

        bool has_value = false;

        explicit Result(ValueType&& value) : value(std::move(value)), has_value(true) {}

        explicit Result(const ValueType& value) : value(value), has_value(true) {}

        explicit Result(ErrorType error) : error(std::move(error)) {}

        explicit Result(const Result<ValueType>& other) = delete;
        Result<ValueType>& operator=(const Result<ValueType>& other) = delete;

        explicit Result(Result<ValueType>&& old) noexcept {
            if(old.has_value) {
                value = std::move(old.value);
                old.value = {};

                has_value = true;
            } else {
                error = std::move(old.error);
                old.error = {};
            }

            old.is_moved = true;
        };

        Result& operator=(Result<ValueType>&& old) noexcept {
            if(old.has_value) {
                value = std::move(old.value);
                has_value = true;

            } else {
                error = std::move(old.error);
            }

            old.is_moved = true;

            return *this;
        };

        ~Result() {
            if(!is_moved) {
                if(has_value) {
                    value.~ValueType();
                } else {
                    error.~ErrorType();
                }
            }
        }

        template <typename FuncType>
        auto map(FuncType&& func) -> Result<decltype(func(value))> {
            using RetVal = decltype(func(value));

            if(has_value) {
                return Result<RetVal>(func(value));
            } else {
                return Result<RetVal>(std::move(error));
            }
        }

        template <typename FuncType>
        auto flat_map(FuncType&& func) -> Result<decltype(func(value).value)> {
            using RetVal = decltype(func(value).value);

            if(has_value) {
                return func(value);
            } else {
                return Result<RetVal>(std::move(error));
            }
        }

        template <typename FuncType>
        void if_present(FuncType&& func) {
            if(has_value) {
                func(value);
            }
        }

        template <typename FuncType>
        void on_error(FuntType&& error_func) const {
            if(!has_value) {
                error_func(error);
            }
        }

        operator bool() const { return has_value; }

        ValueType operator*() { return value; }
    };

    template <typename ValueType>
    Result(ValueType value)->Result<ValueType>;

#define MAKE_ERROR(s, ...) ::ntl::NovaError(fmt::format(fmt(s), __VA_ARGS__).c_str())
} // namespace ntl