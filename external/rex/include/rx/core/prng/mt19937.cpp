#include "rx/core/prng/mt19937.h"
#include "rx/core/hints/unlikely.h"

namespace rx::prng {

static inline constexpr rx_u32 m32(rx_u32 _x) {
  return 0x80000000 & _x;
}

static inline constexpr rx_u32 l31(rx_u32 _x) {
  return 0x7fffffff & _x;
}

static inline constexpr bool odd(rx_u32 _x) {
  return _x & 1;
}

void mt19937::seed(rx_u32 _seed) {
  m_index = 0;
  m_state[0] = _seed;
  for (rx_size i{1}; i < k_size; i++) {
    m_state[i] = 0x6c078965 * (m_state[i - 1] ^ m_state[i - 1] >> 30) + i;
  }
}

rx_u32 mt19937::u32() {
  if (RX_HINT_UNLIKELY(m_index == 0)) {
    generate();
  }

  rx_u32 value{m_state[m_index]};
  value ^= value >> 11;
  value ^= value << 7 & 0x9d2c5680;
  value ^= value << 15 & 0xefc60000;
  value ^= value >> 18;

  if (RX_HINT_UNLIKELY(++m_index == k_size)) {
    m_index = 0;
  }

  return value;
}

void mt19937::generate() {
  rx_u32 i{0};
  rx_u32 y{0};
  auto unroll{[&](rx_u32 _expr) {
    y = m32(m_state[i]) | l31(m_state[i+1]);
    m_state[i] = m_state[_expr] ^ (y >> 1) ^ (0x9908b0df * odd(y));
    i++;
  }};

  // i = [0, 226]
  while (i < k_difference - 1) {
    unroll(i + k_period);
    unroll(i + k_period);
  }

  // i = 256
  unroll((i + k_period) % k_size);

  // i = [227, 622]
  while (i < k_size - 1) {
    unroll(i - k_difference);
    unroll(i - k_difference);
    unroll(i - k_difference);
    unroll(i - k_difference);
    unroll(i - k_difference);
    unroll(i - k_difference);
    unroll(i - k_difference);
    unroll(i - k_difference);
    unroll(i - k_difference);
    unroll(i - k_difference);
    unroll(i - k_difference);
  }

  // i = 623
  y = m32(m_state[k_size - 1]) | l31(m_state[0]);
  m_state[k_size - 1] = m_state[k_period - 1] ^ (y >> 1) ^ (0x9908b0df * odd(y));
}

} // namespace rx::prng
