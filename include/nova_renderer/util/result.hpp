#pragma once

#include <rx/core/optional.h>
#include <rx/core/string.h>

namespace ntl {
    struct NovaError;

    struct NovaError {
        rx::string message = "";

        NovaError* cause = nullptr;

        explicit NovaError(rx::string message);

        NovaError(rx::string message, NovaError* cause);

        [[nodiscard]] rx::string to_string() const;
    };

    inline NovaError operator""_err(const char* str, const rx_size size) { return NovaError(rx::string(str, size)); }

    template <typename ValueType, typename ErrorType = NovaError>
    struct [[nodiscard]] Result {
        union {
            ValueType value;
            ErrorType error;
        };

        bool has_value = false;

        explicit Result(ValueType && value) : value(value), has_value(true) {}

        explicit Result(const ValueType& value) : value(value), has_value(true) {}

        explicit Result(ErrorType error) : error(rx::utility::move(error)) {}

        explicit Result(const Result<ValueType, ErrorType>& other) {
            if(other.has_value) {
                value = other.value;
            } else {
                error = other.error;
            }
        };
        Result& operator=(const Result<ValueType, ErrorType>& other) {
            if(other.has_value) {
                value = other.value;
            } else {
                error = other.error;
            }

            return *this;
        };

        explicit Result(Result<ValueType, ErrorType> && old) noexcept {
            if(old.has_value) {
                value = rx::utility::move(old.value);
                old.value = {};

                has_value = true;
            } else {
                error = rx::utility::move(old.error);
                old.error = {};
            }
        };

        Result& operator=(Result<ValueType, ErrorType>&& old) noexcept {
            if(old.has_value) {
                value = old.value;
                old.value = {};

                has_value = true;
            } else {
                error = old.error;
                old.error = {};
            }

            return *this;
        };

        ~Result() {
            if(has_value) {
                value.~ValueType();
            } else {
                error.~ErrorType();
            }
        }

        const ValueType* operator->() const { return &value; }

        const ValueType& operator*() const { return value; }

        template <typename FuncType>
        auto map(FuncType && func)->Result<decltype(func(value))> {
            using RetVal = decltype(func(value));

            if(has_value) {
                return Result<RetVal>(func(value));
            } else {
                return Result<RetVal>(rx::utility::move(error));
            }
        }

        template <typename FuncType>
        auto flat_map(FuncType && func)->Result<decltype(func(value).value)> {
            using RetVal = decltype(func(value).value);

            if(has_value) {
                return func(value);
            } else {
                return Result<RetVal>(rx::utility::move(error));
            }
        }

        template <typename FuncType>
        void if_present(FuncType && func) {
            if(has_value) {
                func(value);
            }
        }

        template <typename FuncType>
        void on_error(FuncType && error_func) const {
            if(!has_value) {
                error_func(error);
            }
        }

        // ReSharper disable once CppNonExplicitConversionOperator
        operator bool() const { return has_value; }

        ValueType operator*() { return value; }
    };

    template <typename ValueType>
    Result(ValueType value)->Result<ValueType>;

#define MAKE_ERROR(s, ...) ::ntl::NovaError(::rx::string::format(s, __VA_ARGS__))
} // namespace ntl
