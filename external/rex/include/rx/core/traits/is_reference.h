#ifndef RX_CORE_TRAITS_IS_REFERENCE_H
#define RX_CORE_TRAITS_IS_REFERENCE_H

namespace rx::traits {

template<typename T>
inline constexpr const bool is_reference{false};

template<typename T>
inline constexpr const bool is_reference<T&>{true};

template<typename T>
inline constexpr const bool is_reference<T&&>{true};

} // namespace rx::traits

#endif // RX_CORE_TRAITS_IS_REFERENCE_H
