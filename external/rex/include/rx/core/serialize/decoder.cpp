#include <string.h> // memcmp

#include "rx/core/serialize/decoder.h"
#include "rx/core/stream.h"
#include "rx/core/assert.h"

namespace rx::serialize {

decoder::decoder(memory::allocator* _allocator, stream* _stream)
  : m_allocator{_allocator}
  , m_stream{_stream}
  , m_buffer{m_stream, buffer::mode::k_read}
  , m_message{m_allocator}
{
  RX_ASSERT(_stream->can_seek(), "decoder requires seekable stream");
  RX_ASSERT(_stream->can_tell(), "decoder requires tellable stream");

  // Read header and strings.
  RX_ASSERT(read_header(), "failed to read header");
  RX_ASSERT(read_strings(), "failed to read strings");

  // Read data into |m_buffer| for the decoder to begin using.
  RX_ASSERT(m_buffer.read(m_header.data_size), "buffer failed");
}

decoder::~decoder() {
  RX_ASSERT(finalize(), "finalization failed");
}

bool decoder::finalize() {
  if (m_header.string_size) {
    m_strings.fini();
  }
  return true;
}

bool decoder::read_header() {
  auto header = reinterpret_cast<rx_byte*>(&m_header);
  if (m_stream->read(header, sizeof m_header) != sizeof m_header) {
    return error("could not read header");
  }

  // Check fields of the header to see if they're correct.
  if (memcmp(m_header.magic, "REX", 4) != 0) {
    return error("malformed header");
  }

  // Sum of all sections and header should be the same size as the stream.
  rx_u64 size = 0;
  size += sizeof m_header;
  size += m_header.data_size;
  size += m_header.string_size;

  if (size != m_stream->size()) {
    return error("corrupted stream");
  }

  return true;
}

bool decoder::read_strings() {
  // No need to read string table if empty.
  if (m_header.string_size == 0) {
    return true;
  }

  const auto cursor = m_stream->tell();

  // Seek to the strings offset.
  if (!m_stream->seek(m_header.data_size + sizeof m_header, stream::whence::k_set)) {
    return error("seek failed");
  }

  vector<char> strings{m_allocator};
  if (!strings.resize(m_header.string_size, utility::uninitialized{})) {
    return error("out of memory");
  }

  if (!m_stream->read(reinterpret_cast<rx_byte*>(strings.data()), strings.size())) {
    return error("read failed");
  }

  // The last character in the string table is always a null-terminator.
  if (strings.last() != '\0') {
    return error("malformed string table");
  }

  m_strings.init(utility::move(strings));

  // Restore the stream to where we were before we seeked and read in the strings
  if (!m_stream->seek(cursor, stream::whence::k_set)) {
    return error("seek failed");
  }

  return true;
}

bool decoder::read_uint(rx_u64& value_) {
  rx_byte byte;

  auto shift = 0_u64;
  auto value = 0_u64;

  do if (!m_buffer.read_byte(&byte)) {
    return error("unexpected end of stream");
  } else {
    const rx_u64 slice = byte & 0x7f;
    if (shift >= 64 || slice << shift >> shift != slice) {
      return error("ULEB128 value too large");
    }
    value += static_cast<rx_u64>(slice) << shift;
    shift += 7;
  } while (byte >= 0x80);

  value_ = value;

  return true;
}

bool decoder::read_sint(rx_s64& value_) {
  rx_byte byte;

  auto shift = 0_u64;
  auto value = 0_u64;

  do if (!m_buffer.read_byte(&byte)) {
    return error("unexpected end of stream");
  } else {
    value |= (static_cast<rx_u64>(byte & 0x7f) << shift);
    shift += 7;
  } while (byte >= 0x80);

  // Sign extend negative numbers.
  if (shift < 64 && (byte & 0x40)) {
    value |= -1_u64 << shift;
  }

  value_ = static_cast<rx_s64>(value);

  return true;
}

bool decoder::read_bytes(rx_byte* data_, rx_size _size) {
  rx_u64 size = 0;
  if (!read_uint(size)) {
    return false;
  }

  if (static_cast<rx_size>(size) != _size) {
    return error("invalid size");
  }

  if (!m_buffer.read_bytes(data_, _size)) {
    return error("unexpected end of stream");
  }

  return true;
}

bool decoder::read_string(string& result_) {
  rx_u64 index = 0;
  if (!read_uint(index)) {
    return false;
  }

  result_ = (*m_strings.data())[index];
  return true;
}

} // namespace rx::serialize
