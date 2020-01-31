#ifndef RX_CORE_TRAITS_DETECT_H
#define RX_CORE_TRAITS_DETECT_H

namespace rx::traits {

namespace detail {
  template<typename...>
  using empty = void;

  template<typename, template<typename> class, typename = empty<>>
  struct detect {
    static inline bool value = false;
  };

  template<typename T, template<typename> class O>
  struct detect<T, O, empty<O<T>>> {
    static inline bool value = true;
  };
} // namespace detail

template<typename T, template<typename> class O>
inline bool detect = detail::detect<T, O>::value;

} // namespace rx::traits

#endif // RX_CORE_TRAITS_DETECT_H
