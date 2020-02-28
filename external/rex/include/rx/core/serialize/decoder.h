#ifndef RX_CORE_SERIALIZE_DECODER_H
#define RX_CORE_SERIALIZE_DECODER_H
#include "rx/core/serialize/buffer.h"
#include "rx/core/serialize/header.h"

#include "rx/core/string.h"
#include "rx/core/string_table.h"
#include "rx/core/optional.h"

namespace rx {
struct stream;
} // namespace rx

namespace rx::serialize {

struct decoder {
  decoder(stream* _stream);
  decoder(memory::allocator* _allocator, stream* _stream);
  ~decoder();

  // Read unsigned integer value into |value_|.
  [[nodiscard]] bool read_uint(rx_u64& value_);

  // Read signed integer value into |value_|.
  [[nodiscard]] bool read_sint(rx_s64& value_);

  // Read string value into |result_|.
  [[nodiscard]] bool read_string(string& result_);

  // Read |_size| bytes data into |data_|.
  [[nodiscard]] bool read_bytes(rx_byte* data_, rx_size _size);

  const string& message() const &;
  memory::allocator* allocator() const;

private:
  template<typename... Ts>
  bool error(const char* _format, Ts&&... _arguments);

  [[nodiscard]] bool read_header();
  [[nodiscard]] bool read_strings();
  [[nodiscard]] bool finalize();

  memory::allocator* m_allocator;
  stream* m_stream;

  header m_header;
  buffer m_buffer;
  string m_message;
  memory::uninitialized_storage<string_table> m_strings;
};

inline decoder::decoder(stream* _stream)
  : decoder{&memory::g_system_allocator, _stream}
{
}

inline memory::allocator* decoder::allocator() const {
  return m_allocator;
}

inline const string& decoder::message() const & {
  return m_message;
}

template<typename... Ts>
inline bool decoder::error(const char* _format, Ts&&... _arguments) {
  m_message = string::format(_format, utility::forward<Ts>(_arguments)...);
  return false;
}

} // namespace rx::serialize

#endif // RX_CORE_SERIALIZE_DECODER_H
