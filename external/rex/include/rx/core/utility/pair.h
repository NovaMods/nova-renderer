#ifndef RX_CORE_UTILITY_PAIR_H
#define RX_CORE_UTILITY_PAIR_H
#include "rx/core/utility/forward.h"

namespace rx {

template<typename T1, typename T2>
struct pair {
  constexpr pair();
  constexpr pair(const T1& _first, const T2& _second);

  template<typename U1, typename U2>
  constexpr pair(U1&& first_, U2&& second_);

  template<typename U1, typename U2>
  constexpr pair(const pair<U1, U2>& _pair);

  template<typename U1, typename U2>
  constexpr pair(pair<U1, U2>&& pair_);

  T1 first;
  T2 second;
};

template<typename T1, typename T2>
inline constexpr pair<T1, T2>::pair()
  : first{}
  , second{}
{
}

template<typename T1, typename T2>
inline constexpr pair<T1, T2>::pair(const T1& _first, const T2& _second)
  : first{_first}
  , second{_second}
{
}

template<typename T1, typename T2>
template<typename U1, typename U2>
inline constexpr pair<T1, T2>::pair(U1&& first_, U2&& second_)
  : first{utility::forward<U1>(first_)}
  , second{utility::forward<U2>(second_)}
{
}

template<typename T1, typename T2>
template<typename U1, typename U2>
inline constexpr pair<T1, T2>::pair(const pair<U1, U2>& _pair)
  : first{_pair.first}
  , second{_pair.second}
{
}

template<typename T1, typename T2>
template<typename U1, typename U2>
inline constexpr pair<T1, T2>::pair(pair<U1, U2>&& pair_)
  : first{utility::forward<U1>(pair_.first)}
  , second{utility::forward<U2>(pair_.second)}
{
}

template<typename T1, typename T2>
pair(T1, T2) -> pair<T1, T2>;

} // namespace rx

#endif // RX_CORE_UTILITY_PAIR_H
