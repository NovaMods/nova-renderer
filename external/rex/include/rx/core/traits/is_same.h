#ifndef RX_CORE_TRAITS_IS_SAME_H
#define RX_CORE_TRAITS_IS_SAME_H

namespace rx::traits {

template<typename T1, typename T2>
inline constexpr const bool is_same{false};

template<typename T>
inline constexpr const bool is_same<T, T>{true};

} // namespace rx::traits


#endif // RX_CORE_TRAITS_IS_SAME_H
