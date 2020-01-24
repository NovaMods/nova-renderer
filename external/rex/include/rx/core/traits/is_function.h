#ifndef RX_CORE_TRAITS_IS_FUNCTION_H
#define RX_CORE_TRAITS_IS_FUNCTION_H
#include "rx/core/config.h" // __has_feature

namespace rx::traits {

#if __has_feature(__is_function)
// use compiler proxy for faster compilation if supported
template<typename T>
inline constexpr const bool is_function{__is_function(T)};
#else
template<typename T>
inline constexpr const bool is_function{false};

// specialization for regular functions
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts...)>{true};

// specialization for variadic functions
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts..., ...)>{true};

// specialization for function types that have cv-qualifiers
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts...) const>{true};
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts...) volatile>{true};
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts...) const volatile>{true};
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts..., ...) const>{true};
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts..., ...) volatile>{true};
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts..., ...) const volatile>{true};

// specialization for function types that have ref-qualifiers
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts...) &>{true};
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts...) const &>{true};
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts...) volatile &>{true};
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts...) const volatile &>{true};
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts..., ...) &>{true};
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts..., ...) const &>{true};
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts..., ...) volatile &>{true};
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts..., ...) const volatile &>{true};
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts...) &&>{true};
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts...) const &&>{true};
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts...) volatile &&>{true};
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts...) const volatile &&>{true};
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts..., ...) &&>{true};
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts..., ...) const &&>{true};
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts..., ...) volatile &&>{true};
template<typename R, typename... Ts>
inline constexpr const bool is_function<R(Ts..., ...) const volatile &&>{true};

// NOTE(dweiler): there's also noexcept versions of all the above except
// we turn exceptions off which means all functions are implicitly
// noexcept without actually needing to mark them noexcept
#endif

} // namespace rx::traits

#endif // RX_CORE_TRAITS_IS_FUNCTION_H
