#ifndef RX_CORE_TRAITS_IS_ARRAY_H
#define RX_CORE_TRAITS_IS_ARRAY_H
#include "rx/core/types.h" // rx_size

namespace rx::traits {

template<typename T>
inline constexpr const bool is_array{false};

template<typename T>
inline constexpr const bool is_array<T[]>{true};

template<typename T, rx_size E>
inline constexpr const bool is_array<T[E]>{true};

} // namespace rx::traits

#endif // RX_CORE_TRAITS_IS_ARRAY_H
