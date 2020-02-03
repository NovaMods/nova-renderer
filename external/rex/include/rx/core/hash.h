#ifndef RX_CORE_HASH_H
#define RX_CORE_HASH_H
#include "rx/core/types.h"

#include "rx/core/traits/detect.h"
#include "rx/core/traits/underlying_type.h"

#include "rx/core/utility/declval.h"

namespace rx {

template<typename T>
struct hash {
  template<typename U>
  using has_hash = decltype(utility::declval<U>().hash());

  rx_size operator()(const T& _value) const {
    if constexpr (traits::detect<T, has_hash>) {
      // The type has a hash member function we can use.
      return _value.hash();
    } else if constexpr (traits::is_enum<T>) {
      // We can convert the enum to the underlying type and forward to existing
      // integer hash specialization.
      using underlying_type = traits::underlying_type<T>;
      return hash<underlying_type>{}(static_cast<underlying_type>(_value));
    } else {
      static_assert("implement size_t T::hash()");
    }
  }
};

template<>
struct hash<bool> {
  constexpr rx_size operator()(bool _value) const {
    return _value ? 1231 : 1237;
  }
};

template<>
struct hash<rx_u32> {
  constexpr rx_size operator()(rx_u32 _value) const {
    _value = (_value ^ 61) ^ (_value >> 16);
    _value = _value + (_value << 3);
    _value = _value ^ (_value >> 4);
    _value = _value * 0x27D4EB2D;
    _value = _value ^ (_value >> 15);
    return static_cast<rx_size>(_value);
  }
};

template<>
struct hash<rx_s32> {
  constexpr rx_size operator()(rx_s32 value) const {
    return hash<rx_u32>{}(static_cast<rx_u32>(value));
  }
};

template<>
struct hash<rx_u64> {
  constexpr rx_size operator()(rx_u64 _value) const {
    _value = (~_value) + (_value << 21);
    _value = _value ^ (_value >> 24);
    _value = (_value + (_value << 3)) + (_value << 8);
    _value = _value ^ (_value >> 14);
    _value = (_value + (_value << 2)) + (_value << 4);
    _value = _value ^ (_value << 28);
    _value = _value + (_value << 31);
    return static_cast<rx_size>(_value);
  }
};

template<>
struct hash<rx_s64> {
  constexpr rx_size operator()(rx_s64 _value) const {
    return hash<rx_u64>{}(static_cast<rx_u64>(_value));
  }
};

template<>
struct hash<rx_f32> {
  rx_size operator()(rx_f32 _value) const {
    const union { rx_f32 f; rx_u32 u; } cast{_value};
    return hash<rx_u32>{}(cast.u);
  };
};

template<>
struct hash<rx_f64> {
  rx_size operator()(rx_f64 _value) const {
    const union { rx_f64 f; rx_u64 u; } cast{_value};
    return hash<rx_u64>{}(cast.u);
  };
};

template<typename T>
struct hash<T*> {
  constexpr rx_size operator()(T* _value) const {
    if constexpr (sizeof _value == 8) {
      return hash<rx_u64>{}(reinterpret_cast<rx_u64>(_value));
    } else {
      return hash<rx_u32>{}(reinterpret_cast<rx_u32>(_value));
    }
  }
};

inline constexpr rx_size hash_combine(rx_size _hash1, rx_size _hash2) {
  return _hash1 ^ (_hash2 + 0x9E3779B9 + (_hash1 << 6) + (_hash1 >> 2));
}

} // namespace rx

#endif // RX_CORE_HASH_H
