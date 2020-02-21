#include <string.h> // memcpy

#include "rx/core/serialize/buffer.h"
#include "rx/core/stream.h"
#include "rx/core/algorithm/min.h"

namespace rx::serialize {

buffer::buffer(stream* _stream, mode _mode)
  : m_stream{_stream}
  , m_mode{_mode}
  , m_cursor{0}
{
  switch (_mode) {
  case mode::k_read:
    RX_ASSERT(_stream->can_read(), "buffer requires readable stream");
    break;
  case mode::k_write:
    RX_ASSERT(_stream->can_write(), "buffer requires writable stream");
    break;
  }
}

buffer::~buffer() {
  switch (m_mode) {
  case mode::k_read:
    RX_ASSERT(m_cursor == m_length, "data left in buffer");
    break;
  case mode::k_write:
    RX_ASSERT(flush(), "flush failed");
    break;
  }
}

bool buffer::write_byte(rx_byte _byte) {
  if (m_cursor == k_size && !flush()) {
    return false;
  }
  m_buffer[m_cursor++] = _byte;
  return true;
}

bool buffer::read_byte(rx_byte* byte_) {
  if (m_cursor == m_length && !read()) {
    return false;
  }
  *byte_ = m_buffer[m_cursor++];
  return true;
}

bool buffer::write_bytes(const rx_byte* _bytes, rx_size _size) {
  while (_size) {
    if (m_cursor == k_size && !flush()) {
      return false;
    }
    const auto max = algorithm::min(_size, k_size - m_cursor);
    memcpy(m_buffer + m_cursor, _bytes, max);
    m_cursor += max;
    _bytes += max;
    _size -= max;
  }
  return true;
}

bool buffer::read_bytes(rx_byte* bytes_, rx_size _size) {
  while (_size) {
    if (m_cursor == m_length && !read()) {
      return false;
    }
    const auto max = algorithm::min(_size, m_length - m_cursor);
    memcpy(bytes_, m_buffer + m_cursor, max);
    m_cursor += max;
    bytes_ += max;
    _size -= max;
  }
  return true;
}

bool buffer::flush() {
  const auto size = m_cursor;
  const auto bytes = m_stream->write(m_buffer, size);
  m_cursor = 0;
  return bytes == size;
}

bool buffer::read(rx_u64 _max_bytes) {
  _max_bytes = algorithm::min(k_size, _max_bytes);
  const auto bytes = m_stream->read(m_buffer, _max_bytes);
  m_cursor = 0;
  m_length = bytes;
  return m_length != 0;
}

} // namespace rx::serialize
