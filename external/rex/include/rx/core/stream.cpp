#include "rx/core/stream.h"
#include "rx/core/string.h" // utf16_to_utf8

namespace rx {

static vector<rx_byte> convert_text_encoding(vector<rx_byte>&& data_) {
  // Ensure the data contains a null-terminator.
  if (data_.last() != 0) {
    data_.push_back(0);
  }

  const bool utf16_le = data_.size() >= 2 && data_[0] == 0xFF && data_[1] == 0xFE;
  const bool utf16_be = data_.size() >= 2 && data_[0] == 0xFE && data_[1] == 0xFF;

  // UTF-16.
  if (utf16_le || utf16_be) {
    // Remove the BOM.
    data_.erase(0, 2);

    auto contents = reinterpret_cast<rx_u16*>(data_.data());
    const rx_size chars = data_.size() / 2;
    if (utf16_be) {
      // Swap the bytes around in the contents to convert BE to LE.
      for (rx_size i = 0; i < chars; i++) {
        contents[i] = (contents[i] >> 8) | (contents[i] << 8);
      }
    }

    // Determine how many bytes are needed to convert the encoding.
    const rx_size length = utf16_to_utf8(contents, chars, nullptr);

    // Convert UTF-16 to UTF-8.
    vector<rx_byte> result{data_.allocator(), length, utility::uninitialized{}};
    utf16_to_utf8(contents, chars, reinterpret_cast<char*>(result.data()));
    return result;
  } else if (data_.size() >= 3 && data_[0] == 0xEF && data_[1] == 0xBB && data_[2] == 0xBF) {
    // Remove the BOM.
    data_.erase(0, 3);
  }

  return utility::move(data_);
}

rx_u64 stream::read(rx_byte* _data, rx_u64 _size) {
  RX_ASSERT(can_read(), "cannot read");
  return on_read(_data, _size);
}

rx_u64 stream::write(const rx_byte* _data, rx_u32 _size) {
  RX_ASSERT(can_write(), "cannot write");
  return on_write(_data, _size);
}

bool stream::seek(rx_s64 _where, whence _whence) {
  RX_ASSERT(can_seek(), "cannot seek");
  return on_seek(_where, _whence);
}

bool stream::flush() {
  RX_ASSERT(can_flush(), "cannot flush");
  return on_flush();
}

optional<rx_u64> stream::size() {
  if (can_size()) {
    return on_size();
  }
  return nullopt;
}

optional<vector<rx_byte>> read_binary_stream(memory::allocator* _allocator, stream* _stream) {
  if (const auto size = _stream->size()) {
    vector<rx_byte> result = {_allocator, *size, utility::uninitialized{}};
    if (_stream->read(result.data(), *size) == *size) {
      return result;
    }
  }
  return nullopt;
}

optional<vector<rx_byte>> read_text_stream(memory::allocator* _allocator, stream* _stream) {
  if (auto result = read_binary_stream(_allocator, _stream)) {
    // Convert the given byte stream into a compatible UTF-8 encoding. This will
    // introduce a null-terminator, strip Unicode BOMs and convert UTF-16
    // encodings to UTF-8.
    auto data = convert_text_encoding(utility::move(*result));

#if defined(RX_PLATFORM_WINDOWS)
    // Remove all instances of CR from the byte stream.
    auto next = reinterpret_cast<rx_byte*>(memchr(data.data(), '\r', data.size()));

    // Leverage the use of optimized memchr to skip through large swaths of
    // binary data quickly, rather than the more obvious per-byte approach here.
    while (next) {
      const rx_ptrdiff index = next - data.data();
      data.erase(index, index + 1);
      next = reinterpret_cast<rx_byte*>(memchr(next + 1, '\r', data.size() - index));
    }
#endif

    return data;
  }

  return nullopt;
}

} // namespace rx
