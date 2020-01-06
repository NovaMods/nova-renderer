#ifndef RX_CORE_MATH_SIGN_H
#define RX_CORE_MATH_SIGN_H

namespace rx::math {

template<typename T>
inline constexpr T sign(T _value) {
    return T(T{0} < _value) - T(_value < T{0});
}

} // namespace rx::math

#endif // RX_CORE_MATH_SIGN_H