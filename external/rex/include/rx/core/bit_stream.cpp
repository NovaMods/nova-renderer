#include <string.h> // memmove, memset

#include "rx/core/bit_stream.h"
#include "rx/core/stream.h"
#include "rx/core/assert.h"

#include "rx/core/algorithm/min.h"

namespace rx {

bit_stream::bit_stream(stream* _stream)
  : m_stream{_stream}
{
  const bool can_read = m_stream->can_read();
  const bool can_write = m_stream->can_write();

  RX_ASSERT(can_read != can_write,
    "stream can be read or write only, not both");

  if (can_read) {
    m_buffer.size = 0;
    m_buffer.rd = 0;
  } else {
    m_buffer.size = sizeof m_buffer.data;
    m_buffer.wr = 0;
  }
}

bit_stream::~bit_stream() {
  if (!m_stream->can_write()) {
    return;
  }

  // Zero pad until we have a full a byte.
  const rx_size remainder = m_buffer.wr % 8;
  if (remainder) {
    [[maybe_unused]] bool result = write(0_u64, remainder);
    RX_ASSERT(result, "zero pad failed");
  }

  [[maybe_unused]] bool result = flush();
  RX_ASSERT(result, "flush failed");
}

bool bit_stream::fetch() {
  // The amount of bytes read from the buffer so far.
  const rx_size bytes = m_buffer.rd >> 3;

  // Shift the contents of the buffer backwards to erase the first |bytes|,
  // bytes as these bytes have already been read.
  memmove(m_buffer.data, m_buffer.data + bytes, m_buffer.size - bytes);

  // Shift the read cursor backwards too.
  m_buffer.rd -= bytes * 8;

  // The buffer has become smaller by |bytes| bytes.
  m_buffer.size -= bytes;

  // The amount of free space in bytes on the tail end of the buffer.
  const rx_size free_space = sizeof m_buffer.data - m_buffer.size;

  // Read in |free_space| bytes at the tail end of the buffer.
  const rx_u64 read_bytes =
    m_stream->read(m_buffer.data + m_buffer.size, free_space);

  // Could not read anymore bytes from the stream.
  if (read_bytes == 0) {
    return false;
  }

  // How ever many bytes we read from the stream is added to our buffer.
  m_buffer.size += read_bytes;

  return true;
}

bool bit_stream::flush() {
  // The amount of bytes written to the buffer so far.
  const rx_size bytes = m_buffer.wr >> 3;

  // Write |bytes| bytes from |m_buffer| to the stream.
  const rx_u64 wrote_bytes = m_stream->write(m_buffer.data, bytes);

  // This is a fatal, unrecoverable error.
  if (wrote_bytes != bytes) {
    return false;
  }

  // Shift the contents of the buffer backwards to erase the first |bytes|,
  // bytes as these bytes have already been written.
  memmove(m_buffer.data, m_buffer.data + bytes, m_buffer.size - bytes);

  // Shift the write cursor backwards too.
  m_buffer.wr -= bytes * 8;

  return true;
}

bool bit_stream::read(rx_u64& data_, rx_size _bits) {
  RX_ASSERT(m_stream->can_read(), "bit_stream isn't readable");
  RX_ASSERT(_bits <= sizeof data_ * 8, "too many bits");

  const rx_size available_bits = m_buffer.size * 8 - m_buffer.rd;

  // Not enough bits to read.
  if (_bits > available_bits) {
    if (fetch()) {
      return read(data_, _bits);
    } else {
      return false;
    }
  }

  // Read data bit-by-bit into |value| from |m_buffer|.
  rx_u64 value = 0;
  for (rx_size i = 0; i < _bits; ) {
    const rx_size byte_offset = m_buffer.rd >> 3;
    const rx_size bit_offset = m_buffer.rd & 7;
    const rx_size max = algorithm::min(_bits - i, 8 - bit_offset);
    const rx_u32 mask = ~(0xff << max);
    const rx_u32 read = (m_buffer.data[byte_offset] >> bit_offset) & mask;
    value |= read << i;

    m_buffer.rd += max;
    i += max;
  }

  data_ = value;
  return true;
}

bool bit_stream::write(rx_u64 _data, rx_size _bits) {
  RX_ASSERT(m_stream->can_write(), "bit_stream isn't writable");
  RX_ASSERT(_bits <= sizeof _data * 8, "too many bits");

  const rx_size available_bits = m_buffer.size * 8 - m_buffer.wr;

  if (_bits > available_bits) {
    if (flush()) {
      return write(_data, _bits);
    } else {
      return false;
    }
  }

  // Write |_data| bit-by-bit to |m_buffer|.
  for (rx_size i = 0; i < _bits; ) {
    const rx_size byte_offset = m_buffer.wr >> 3;
    const rx_size bit_offset = m_buffer.wr & 7;
    const rx_size max = algorithm::min(_bits - i, 8 - bit_offset);
    const rx_u32 mask = ~(0xff << max);
    const rx_u32 write = _data & mask;

    m_buffer.data[byte_offset] =
      (m_buffer.data[byte_offset] & ~(mask << bit_offset))
      | (write << bit_offset);

    _data >>= max;
    m_buffer.wr += max;
    i += max;
  }

  return true;
}

} // namespace rx
