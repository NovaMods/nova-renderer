#ifndef RX_CORE_FILESYSTEM_MMAP_H
#define RX_CORE_FILESYSTEM_MMAP_H
#include "rx/core/stream.h"

namespace rx::filesystem {

struct mmap
  : stream
{
  static constexpr rx_u32 k_stream_flags = stream::k_tell & stream::k_seek;
  static constexpr rx_u32 k_read_flags = k_stream_flags & stream::k_read;
  static constexpr rx_u32 k_write_flags = k_stream_flags & stream::k_write;

  constexpr mmap(rx_byte* _data, rx_size _size);
  constexpr mmap(const rx_byte* _data, rx_size _size);

  virtual rx_u64 on_read(rx_byte* _data, rx_u64 _size);
  virtual rx_u64 on_write(const rx_byte* _data, rx_u64 _size);
  virtual bool on_seek(rx_s64 _where, whence _whence);
  virtual bool on_flush();
  virtual rx_u64 on_tell();

private:
  rx_u64 readable_bytes() const;
  rx_u64 writable_bytes() const;

  rx_byte* m_data;
  rx_size m_size;

  const rx_byte* m_rd;
  rx_byte* m_wr;
};

inline constexpr mmap::mmap(rx_byte* _data, rx_size _size)
  : stream{k_write_flags}
  , m_data{_data}
  , m_size{_size}
  , m_rd{nullptr}
  , m_wr{_data}
{
}

inline constexpr mmap::mmap(const rx_byte* _data, rx_size _size)
  : stream{k_read_flags}
  , m_data{const_cast<rx_byte*>(_data)}
  , m_size{_size}
  , m_rd{_data}
  , m_wr{nullptr}
{
}

inline rx_u64 mmap::readable_bytes() const {
  return m_rd ? m_rd - (m_data + m_size) : 0;
}

inline rx_u64 mmap::writable_bytes() const {
  return m_wr ? m_wr - (m_data + m_size) : 0;
}

} // namespace rx::filesystem

#endif // RX_CORE_FILESYSTEM_MMAP_H
