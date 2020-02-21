#ifndef RX_CORE_SERIALIZE_BUFFER_H
#define RX_CORE_SERIALIZE_BUFFER_H
#include "rx/core/types.h"

namespace rx {
  struct stream;
}

namespace rx::serialize {

struct buffer {
  static constexpr rx_size k_size = 4096;

  enum class mode : rx_u8 {
    k_read,
    k_write
  };

  buffer(stream* m_stream, mode _mode);
  ~buffer();

  [[nodiscard]] bool write_byte(rx_byte _byte);
  [[nodiscard]] bool write_bytes(const rx_byte* _bytes, rx_size _size);

  [[nodiscard]] bool read_byte(rx_byte* byte_);
  [[nodiscard]] bool read_bytes(rx_byte* bytes_, rx_size _size);

  [[nodiscard]] bool read(rx_u64 _at_most = k_size);
  [[nodiscard]] bool flush();

private:
  stream* m_stream;
  mode m_mode;

  rx_byte m_buffer[k_size];
  rx_size m_cursor;
  rx_size m_length;
};

} // namespace rx::serialize

#endif // RX_CORE_SERIALIZE_BUFFER_H
