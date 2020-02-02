#ifndef RX_CORE_FILESYSTEM_MMAP_H
#define RX_CORE_FILESYSTEM_MMAP_H
#include "rx/core/stream.h"

namespace rx::filesystem {

struct mmap
  : stream
{
  constexpr mmap(rx_byte* _data, rx_size _size);
  constexpr mmap(const rx_byte* _data, rx_size _size);

  virtual rx_u64 read(rx_byte* _data, rx_u64 _size);
  virtual rx_u64 write(const rx_byte* _data, rx_u64 _size);
  virtual bool seek(rx_s64 _where, whence _whence);
  virtual bool flush();
  virtual optional<rx_u64> size();

private:
  rx_u64 readable_bytes() const;
  rx_u64 writable_bytes() const;

  rx_byte* m_data;
  rx_size m_size;

  const rx_byte* m_rd;
  rx_byte* m_wr;
};

inline constexpr mmap::mmap(rx_byte* _data, rx_size _size)
  : m_data{_data}
  , m_size{_size}
  , m_rd{nullptr}
  , m_wr{_data}
{
}

inline constexpr mmap::mmap(const rx_byte* _data, rx_size _size)
  : m_data{const_cast<rx_byte*>(_data)}
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
