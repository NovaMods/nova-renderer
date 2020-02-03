#include <string.h> // strstr, strlen, memcpy

#include "rx/core/string_table.h"
#include "rx/core/string.h"

#include "rx/core/hints/unlikely.h"

namespace rx {

string_table::string_table(memory::allocator* _allocator, const char* _data, rx_size _size)
  : m_data{_allocator, _size}
{
  RX_ASSERT(_data[_size] == '\0', "missing null-terminator");
  memcpy(m_data.data(), _data, _size);
}

optional<rx_size> string_table::find(const char* _string) const {
  if (RX_HINT_UNLIKELY(m_data.is_empty())) {
    return nullopt;
  }

  if (const char* search = strstr(m_data.data(), _string)) {
    return static_cast<rx_size>(search - m_data.data());
  }
  return nullopt;
}

optional<rx_size> string_table::add(const char* _string, rx_size _size) {
  const rx_size index = m_data.size();
  const rx_size total = _size + 1;
  if (m_data.resize(index + total, utility::uninitialized{})) {
    memcpy(m_data.data() + index, _string, total);
    return index;
  }
  return nullopt;
}

optional<rx_size> string_table::insert(const char* _string, rx_size _size) {
  if (auto search = find(_string)) {
    return *search;
  }
  return add(_string, _size);
}

optional<rx_size> string_table::insert(const char* _string) {
  return insert(_string, strlen(_string));
}

optional<rx_size> string_table::insert(const string& _string) {
  return insert(_string.data(), _string.size());
}

} // namespace rx
