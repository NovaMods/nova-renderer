#ifndef RX_CORE_TRAITS_IS_TRIVIALLY_COPYABLE
#define RX_CORE_TRAITS_IS_TRIVIALLY_COPYABLE

namespace rx::traits {

template<typename T>
inline constexpr const bool is_trivially_copyable{__is_trivially_copyable(T)};

} // namespace rx::traits

#endif // RX_CORE_TRAITS_IS_TRIVIALLY_COPYABLE
