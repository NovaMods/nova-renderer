#ifndef RX_CORE_TRAITS_IS_TRIVIALLY_DESTRUCTIBLE_H
#define RX_CORE_TRAITS_IS_TRIVIALLY_DESTRUCTIBLE_H

namespace rx::traits {

template<typename T>
inline constexpr const bool is_trivially_destructible{__has_trivial_destructor(T)};

} // namespace rx::traits

#endif // RX_CORE_TRAITS_IS_TRIVIALLY_DESTRUCTIBLE_H
