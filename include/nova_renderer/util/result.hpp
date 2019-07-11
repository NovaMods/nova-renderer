#pragma once

#include <EASTL/functional.h>
#include <EASTL/memory.h>
#include <EASTL/string.h>

#include <fmt/format.h>

#include "utils.hpp"
#include <EASTL/unique_ptr.h>

namespace nova::renderer {
    struct NOVA_API NovaError {
        eastl::string message = "";

        eastl::unique_ptr<NovaError> cause;

        NovaError() = default;

        explicit NovaError(const eastl::string& message);

        NovaError(const eastl::string& message, NovaError cause);

        [[nodiscard]] eastl::string to_string() const;
    };

    inline NOVA_API NovaError operator""_err(const char* str, std::size_t size) { return NovaError(eastl::string(str, size)); }

    template <typename ValueType>
    struct NOVA_API Result {
        union {
            ValueType value;
            NovaError error;
        };

        bool has_value = false;

        explicit Result(ValueType&& value) : value(value), has_value(true) {}

        explicit Result(const ValueType& value) : value(value), has_value(true) {}

        explicit Result(NovaError error) : error(eastl::move(error)) {}

        Result(const Result<ValueType>& other) = delete;
        Result<ValueType>& operator=(const Result<ValueType>& other) = delete;

        Result(Result<ValueType>&& old) noexcept {
            if(old.has_value) {
                value = eastl::move(old.value);
                old.value = {};

                has_value = true;
            } else {
                error = eastl::move(old.error);
                old.error = {};
            }
        };

        Result<ValueType>& operator=(Result<ValueType>&& old) noexcept {
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
                error.~NovaError();
            }
        }

        template <typename FuncType>
        auto map(FuncType&& func) -> Result<decltype(func(value))> {
            using RetVal = decltype(func(value));

            if(has_value) {
                return Result<RetVal>(func(value));
            } else {
                return Result<RetVal>(eastl::move(error));
            }
        }

        template <typename FuncType>
        auto flat_map(FuncType&& func) -> Result<decltype(func(value).value)> {
            using RetVal = decltype(func(value).value);

            if(has_value) {
                return func(value);
            } else {
                return Result<RetVal>(eastl::move(error));
            }
        }

        template <typename FuncType>
        void if_present(FuncType&& func) {
            if(has_value) {
                func(value);
            }
        }

        void on_error(eastl::function<void(const NovaError&)> error_func) const {
            if(!has_value) {
                error_func(error);
            }
        }

        operator bool() const { return has_value; }

        ValueType operator*() { return value; }
    };

    template <typename ValueType>
    Result(ValueType value)->Result<ValueType>;

#define MAKE_ERROR(s, ...) NovaError(fmt::format(fmt(s), __VA_ARGS__).c_str())
} // namespace nova::renderer