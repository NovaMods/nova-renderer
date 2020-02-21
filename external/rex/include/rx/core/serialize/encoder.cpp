#include <string.h> // memcpy

#include "rx/core/serialize/encoder.h"
#include "rx/core/stream.h"

namespace rx::serialize {

encoder::encoder(memory::allocator* _allocator, stream* _stream)
  : m_allocator{_allocator}
  , m_stream{_stream}
  , m_buffer{m_stream, buffer::mode::k_write}
  , m_message{m_allocator}
  , m_strings{m_allocator}
{
  RX_ASSERT(m_stream->can_seek(), "encoder requires seekable stream");
  RX_ASSERT(m_stream->can_tell(), "encoder requires tellable stream");

  // Write out the default header, we'll seek back to patch it later.
  RX_ASSERT(write_header(), "failed to write header");
}

encoder::~encoder() {
  RX_ASSERT(finalize(), "finalization failed");
}

bool encoder::finalize() {
  // Flush anything remaining data in |m_buffer| out to |m_stream|.
  if (!m_buffer.flush()) {
    return error("flush failed");
  }

  // Update header fields.
  m_header.data_size = m_stream->tell() - sizeof m_header;
  m_header.string_size = m_strings.size();

  // Write out string table as the final thing in the stream.
  const auto string_table_data = reinterpret_cast<const rx_byte*>(m_strings.data());
  const auto string_table_size = m_strings.size();
  if (m_stream->write(string_table_data, string_table_size) != string_table_size) {
    return error("write failed");
  }

  // Seek to the beginning of the stream to update the header.
  if (!m_stream->seek(0, stream::whence::k_set)) {
    return error("seek failed");
  }

  return write_header();
}

bool encoder::write_header() {
  const auto header_data = reinterpret_cast<const rx_byte*>(&m_header);
  const auto output_size = m_stream->write(header_data, sizeof m_header);
  if (output_size != sizeof m_header) {
    return error("write failed");
  }
  return true;
}

bool encoder::write_string(const char* _string, rx_size _size) {
  if (_string[_size] != '\0') {
    return error("string isn't null-terminated");
  }

  if (auto insert = m_strings.insert(_string, _size)) {
    return write_uint(*insert);
  }

  return false;
}

bool encoder::write_bytes(const rx_byte* _data, rx_size _size) {
  // Raw byte arrays are prefixed with a ULEB128 encoded size prefix.
  if (!write_uint(_size)) {
    return false;
  }

  if (!m_buffer.write_bytes(_data, _size)) {
    return error("write failed");
  }

  return true;
}

bool encoder::write_uint(rx_u64 _value) {
  // Encode |_value| using ULEB128 encoding.
  do {
    const rx_byte byte = _value & 0x7f;

    _value >>= 7;

    if (!m_buffer.write_byte(_value ? (byte | 0x80) : byte)) {
      return error("write failed");
    }
  } while (_value);

  return true;
}

bool encoder::write_sint(rx_s64 _value) {
  // Encode |_value| using SLEB128 encoding.
  bool more;

  do {
    const rx_byte byte = _value & 0x7f;
    const rx_byte test = byte & 0x40;

    // This assumes signed shift behaves as arithmetic right shift.
    _value >>= 7;

    more = !((_value == 0 && test == 0) || (_value == -1 && test != 0));

    if (!m_buffer.write_byte(more ? (byte | 0x80) : byte)) {
      return error("write failed");
    }
  } while (more);

  return true;
}

} // namespace rx::serialize
