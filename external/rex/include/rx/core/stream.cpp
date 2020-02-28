#include <limits.h> // UCHAR_MAX
#include <string.h> // memmove

#include "rx/core/stream.h"
#include "rx/core/string.h" // utf16_to_utf8

#include "rx/core/hints/may_alias.h"

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

rx_u64 stream::write(const rx_byte* _data, rx_u64 _size) {
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

rx_u64 stream::tell() {
  RX_ASSERT(can_tell(), "cannot tell");
  return on_tell();
}

rx_u64 stream::size() {
  const auto cursor = tell();

  if (!seek(0, whence::k_end)) {
    return 0;
  }

  const auto result = tell();
  if (!seek(cursor, whence::k_set)) {
    return 0;
  }

  return result;
}

optional<vector<rx_byte>> read_binary_stream(memory::allocator* _allocator, stream* _stream) {
  if (_stream->can_seek() && _stream->can_tell()) {
    const auto size = _stream->size();
    vector<rx_byte> result = {_allocator, size, utility::uninitialized{}};
    if (_stream->read(result.data(), size) == size) {
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
    // Quickly scan through word at a time |_src| for CR.
    auto scan = [](const void* _src, rx_size _size) {
      static constexpr auto k_ss = sizeof(rx_size);
      static constexpr auto k_align = k_ss - 1;
      static constexpr auto k_ones = -1_z / UCHAR_MAX; // All bits set.
      static constexpr auto k_highs = k_ones * (UCHAR_MAX / 2 + 1); // All high bits set.
      static constexpr auto k_c = static_cast<const rx_byte>('\r');
      static constexpr auto k_k = k_ones * k_c;

      auto has_zero = [&](rx_size _value) {
        return _value - k_ones & (~_value) & k_highs;
      };

      // Search for CR until |s| is aligned on |k_align| alignment.
      auto s = reinterpret_cast<const rx_byte*>(_src);
      auto n = _size;
      for (; (reinterpret_cast<rx_uintptr>(s) & k_align) && n && *s != k_c; s++, n--);

      // Do checks for CR word at a time, stopping at word containing CR.
      if (n && *s != k_c) {
        // Need to typedef with an alias type since we're breaking strict
        // aliasing, let the compiler know.
        typedef rx_size RX_HINT_MAY_ALIAS word_type;

        // Scan word at a time, stopping at word containing first |k_c|.
        auto w = reinterpret_cast<const word_type*>(s);
        for (; n >= k_ss && !has_zero(*w ^ k_k); w++, n -= k_ss);
        s = reinterpret_cast<const rx_byte*>(w);
      }

      // Handle trailing bytes to determine where in word |k_c| is.
      for (; n && *s != k_c; s++, n--);

      return n ? s : nullptr;
    };

    const rx_byte* src = data.data();
    rx_byte* dst = data.data();
    rx_size size = data.size();
    auto next = scan(src, size);
    if (!next) {
      // Contents do not contain any CR.
      return data;
    }

    // Remove all instances of CR from the byte stream using instruction-level
    // parallelism.
    //
    // Note that the very first iteration will always have |src| == |dst|, so
    // optimize away the initial memmove here as nothing needs to be moved at
    // the beginning.
    const auto length = next - src;
    const auto length_plus_one = length + 1;
    dst += length;
    src += length_plus_one;
    size -= length_plus_one;

    // Subsequent iterations will move contents forward.
    while ((next = scan(src, size))) {
      const auto length = next - src;
      const auto length_plus_one = length + 1;
      memmove(dst, src, length);
      dst += length;
      src += length_plus_one;
      size -= length_plus_one;
    }

    // Ensure the result is null-terminated after all those moves.
    *dst++ = '\0';

    // Respecify the size of storage after removing all those CRs.
    data.resize(dst - data.data());
#endif
    return data;
  }

  return nullopt;
}

} // namespace rx
