#ifndef RX_CORE_TRAITS_DETECT_H
#define RX_CORE_TRAITS_DETECT_H

namespace rx::traits {

namespace detail {
  template<typename...>
  using empty = void;

  template<typename, template<typename> class, typename = empty<>>
  struct detect {
    static inline constexpr const bool value = false;
  };

  template<typename T, template<typename> class O>
  struct detect<T, O, empty<O<T>>> {
    static inline constexpr const bool value = true;
  };
} // namespace detail

template<typename T, template<typename> class O>
inline constexpr const bool detect = detail::detect<T, O>::value;

} // namespace rx::traits

#endif // RX_CORE_TRAITS_DETECT_H
