#ifndef RX_CORE_SERIALIZE_ENCODER_H
#define RX_CORE_SERIALIZE_ENCODER_H
#include "rx/core/serialize/header.h"
#include "rx/core/serialize/buffer.h"

#include "rx/core/string.h"
#include "rx/core/string_table.h"

namespace rx {
struct stream;
} // namespace rx

namespace rx::serialize {

struct encoder {
  encoder(stream* _stream);
  encoder(memory::allocator* _allocator, stream* _stream);
  ~encoder();

  // Write unsigned integer value |_value|.
  [[nodiscard]] bool write_uint(rx_u64 _value);

  // Write signed integer value |_value|.
  [[nodiscard]] bool write_sint(rx_s64 _value);

  // Multiple overloads for writing strings.
  [[nodiscard]] bool write_string(const char* _string, rx_size _size);
  [[nodiscard]] bool write_string(const string& _string);
  template<rx_size E>
  [[nodiscard]] bool write_string(const char (&_string)[E]);

  // Write |_size| bytes from |_data|.
  [[nodiscard]] bool write_bytes(const rx_byte* _data, rx_size _size);

  const string& message() const &;
  memory::allocator* allocator() const;

private:
  template<typename... Ts>
  bool error(const char* _format, Ts&&... _arguments);

  [[nodiscard]] bool write_header();
  [[nodiscard]] bool finalize();

  memory::allocator* m_allocator;
  stream* m_stream;

  header m_header;
  buffer m_buffer;
  string m_message;
  string_table m_strings;
};

inline encoder::encoder(stream* _stream)
  : encoder{&memory::g_system_allocator, _stream}
{
}

template<rx_size E>
inline bool encoder::write_string(const char (&_string)[E]) {
  return write_string(_string, E - 1);
}

inline bool encoder::write_string(const string& _string) {
  return write_string(_string.data(), _string.size());
}

inline memory::allocator* encoder::allocator() const {
  return m_allocator;
}

inline const string& encoder::message() const & {
  return m_message;
}

template<typename... Ts>
inline bool encoder::error(const char* _format, Ts&&... _arguments) {
  m_message = string::format(_format, utility::forward<Ts>(_arguments)...);
  return false;
}

} // namespace rx::serialzie

#endif // RX_CORE_SERIALIZE_ENCODER_H
