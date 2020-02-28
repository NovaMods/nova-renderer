#ifndef RX_CORE_SERIALIZE_HEADER_H
#define RX_CORE_SERIALIZE_HEADER_H
#include "rx/core/types.h"

namespace rx::serialize {

struct header {
  constexpr header();

  // The magic string, always "REX".
  rx_byte magic[4];

  // The serialization version number.
  rx_u32 version;

  // The size of the data and string tables, respectively.
  rx_u64 data_size;
  rx_u64 string_size;
};

inline constexpr header::header()
  : magic{'R', 'E', 'X', '\0'}
  , version{0}
  , data_size{0}
  , string_size{0}
{
}

} // namespace rx::serialize

#endif // RX_CORE_SERIALIZE_HEADER_H
