#include <string.h> // memcpy

#include "rx/core/filesystem/mmap.h"
#include "rx/core/algorithm/min.h"

namespace rx::filesystem {

mmap::mmap(rx_byte* _data, rx_size _size)
  : m_data{_data}
  , m_size{_size}
  , m_rd{nullptr}
  , m_wr{_data}
{
}

mmap::mmap(const rx_byte* _data, rx_size _size)
  : m_data{const_cast<rx_byte*>(_data)}
  , m_size{_size}
  , m_rd{_data}
  , m_wr{nullptr}
{
}

rx_u64 mmap::read(rx_byte* _data, rx_u64 _size) {
  const rx_u64 bytes{algorithm::min(_size, readable_bytes())};
  memcpy(_data, m_rd, bytes);
  m_rd += bytes;
  return bytes;
}

rx_u64 mmap::write(const rx_byte* _data, rx_u64 _size) {
  const rx_u64 bytes{algorithm::min(_size, writable_bytes())};
  memcpy(m_wr, _data, bytes);
  m_wr += bytes;
  return bytes;
}

bool mmap::seek(rx_u64 _where) {
  if (_where >= m_size) {
    return false;
  }

  if (m_rd) {
    m_rd = m_data + _where;
  } else {
    m_wr = m_data + _where;
  }

  return true;
}

bool mmap::flush() {
  return true;
}

optional<rx_u64> mmap::size() {
  return m_size;
}

} // namespace rx::filesystem
