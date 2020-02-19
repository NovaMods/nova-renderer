#include <string.h> // memcpy

#include "rx/core/vector.h"

namespace rx::detail {

void copy(void *RX_HINT_RESTRICT dst_, const void* RX_HINT_RESTRICT _src, rx_size _size) {
  memcpy(dst_, _src, _size);
}

} // namespace rx
