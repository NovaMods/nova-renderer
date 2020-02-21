#ifndef RX_CORE_STREAM_H
#define RX_CORE_STREAM_H
#include "rx/core/vector.h"
#include "rx/core/optional.h"

#include "rx/core/hints/empty_bases.h"

namespace rx {

struct string;

struct RX_HINT_EMPTY_BASES stream
   : concepts::no_copy
{
  // Stream flags.
  enum : rx_u32 {
    k_read  = 1 << 0,
    k_write = 1 << 1,
    k_tell  = 1 << 2,
    k_seek  = 1 << 3,
    k_flush = 1 << 4
  };

  constexpr stream(rx_u32 _flags);
  stream(stream&& stream_);
  ~stream();

  enum class whence {
    k_set,     // Beginning of stream.
    k_current, // Current position
    k_end      // End of stream.
  };

  [[nodiscard]] rx_u64 read(rx_byte* _data, rx_u64 _size);
  [[nodiscard]] rx_u64 write(const rx_byte* _data, rx_u64 _size);
  [[nodiscard]] bool seek(rx_s64 _where, whence _whence);
  [[nodiscard]] bool flush();

  rx_u64 tell();
  rx_u64 size();

  // Query the support of features on the given stream.
  constexpr bool can_read() const;
  constexpr bool can_write() const;
  constexpr bool can_tell() const;
  constexpr bool can_seek() const;
  constexpr bool can_flush() const;

  // The following functions below are what streams must implement. When a
  // stream cannot support a given feature, it should just provide a stub
  // function and ignore the flag when constructing this base class.

  // Read |_size| bytes from stream into |_data|.
  virtual rx_u64 on_read(rx_byte* _data, rx_u64 _size) = 0;

  // Write |_size| bytes from |_data| into stream.
  virtual rx_u64 on_write(const rx_byte* _data, rx_u64 _size) = 0;

  // Seek to |_where| in stream relative to |_whence|.
  virtual bool on_seek(rx_s64 _where, whence _whence) = 0;

  // Flush any buffered contents in the stream out.
  virtual bool on_flush() = 0;

  // Where we are in the stream.
  virtual rx_u64 on_tell() = 0;

  // The name of the stream.
  virtual const string& name() const & = 0;

private:
  rx_u32 m_flags;
};

inline constexpr stream::stream(rx_u32 _flags)
  : m_flags{_flags}
{
}

inline stream::stream(stream&& stream_)
  : m_flags{stream_.m_flags}
{
  stream_.m_flags = 0;
}

inline stream::~stream() {
  // Must be declared because virtual.
}

bool inline constexpr stream::can_read() const {
  return m_flags & k_read;
}

bool inline constexpr stream::can_write() const {
  return m_flags & k_write;
}

bool inline constexpr stream::can_tell() const {
  return m_flags & k_tell;
}

bool inline constexpr stream::can_seek() const {
  return m_flags & k_seek;
}

bool inline constexpr stream::can_flush() const {
  return m_flags & k_flush;
}

optional<vector<rx_byte>> read_binary_stream(memory::allocator* _allocator, stream* _stream);
optional<vector<rx_byte>> read_text_stream(memory::allocator* _allocator, stream* _stream);

} // namespace rx

#endif // RX_CORE_STREAM_H
