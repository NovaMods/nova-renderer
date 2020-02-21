#include <string.h> // memcpy

#include "rx/core/filesystem/mmap.h"
#include "rx/core/algorithm/min.h"

namespace rx::filesystem {

rx_u64 mmap::on_read(rx_byte* _data, rx_u64 _size) {
  const rx_u64 bytes = algorithm::min(_size, readable_bytes());
  memcpy(_data, m_rd, bytes);
  m_rd += bytes;
  return bytes;
}

rx_u64 mmap::on_write(const rx_byte* _data, rx_u64 _size) {
  const rx_u64 bytes = algorithm::min(_size, writable_bytes());
  memcpy(m_wr, _data, bytes);
  m_wr += bytes;
  return bytes;
}

bool mmap::on_seek(rx_s64 _where, whence _whence) {
  auto cursor = reinterpret_cast<rx_uintptr>(m_rd ? m_rd : m_wr);

  switch (_whence) {
  case whence::k_current:
    cursor += _where;
    break;
  case whence::k_end:
    cursor = reinterpret_cast<rx_uintptr>(m_data + m_size) + _where;
    break;
  case whence::k_set:
    cursor = reinterpret_cast<rx_uintptr>(m_data + _where);
    break;
  }

  const auto beg = reinterpret_cast<rx_uintptr>(m_data);
  if (cursor >= beg && cursor < beg + m_size) {
    if (m_rd) {
      m_rd = reinterpret_cast<const rx_byte*>(cursor);
    } else {
      m_wr = reinterpret_cast<rx_byte*>(cursor);
    }
    return true;
  }

  return false;
}

bool mmap::on_flush() {
  return true;
}

rx_u64 mmap::on_tell() {
  return m_rd ? m_rd - m_data : m_wr - m_data;
}

} // namespace rx::filesystem
