#ifndef RX_CORE_PRNG_MT19937_H
#define RX_CORE_PRNG_MT19937_H
#include "rx/core/types.h"
#include "rx/core/utility/nat.h"

namespace rx::prng {

struct mt19937 {
  constexpr mt19937();

  void seed(rx_u32 _seed);

  rx_u32 u32();
  rx_u64 u64();

  rx_f32 f32();
  rx_f64 f64();

private:
  static constexpr const rx_size k_size{624};
  static constexpr const rx_size k_period{397};
  static constexpr const rx_size k_difference{k_size - k_period};
  static constexpr const rx_u32 k_max{0xffffffff_u32};
  void generate();

  union {
    utility::nat m_nat;
    rx_u32 m_state[k_size];
  };
  rx_size m_index;
};

inline constexpr mt19937::mt19937()
  : m_nat{}
  , m_index{0}
{
}

inline rx_u64 mt19937::u64() {
  return static_cast<rx_u64>(u32()) << 32_u64 | u32();
}

inline rx_f32 mt19937::f32() {
  return static_cast<rx_f32>(u32()) / k_max;
}

inline rx_f64 mt19937::f64() {
  return static_cast<rx_f64>(u32()) / k_max;
}

};

#endif // RX_CORE_PRNG_MT19937_H
