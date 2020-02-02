#ifndef RX_CORE_STRING_TABLE_H
#define RX_CORE_STRING_TABLE_H
#include "rx/core/vector.h"
#include "rx/core/optional.h"

namespace rx {

struct string;

struct string_table {
  constexpr string_table();
  constexpr string_table(memory::allocator* _allocator);

  // Construct a string table from raw string data.
  string_table(vector<char>&& data_);
  string_table(memory::allocator* _allocator, const char* _data, rx_size _size);

  string_table(string_table&& string_table_);
  string_table(const string_table& _string_table);

  string_table& operator=(string_table&& string_table_);
  string_table& operator=(const string_table& _string_table);

  optional<rx_size> insert(const char* _string, rx_size _length);
  optional<rx_size> insert(const char* _string);
  optional<rx_size> insert(const string& _string);

  const char* operator[](rx_size _index) const;

  const char* data() const;
  rx_size size() const;

  void clear();

  memory::allocator* allocator() const;

private:
  optional<rx_size> find(const char* _string) const;
  optional<rx_size> add(const char* _string, rx_size _size);

  vector<char> m_data;
};

inline constexpr string_table::string_table()
  : string_table{&memory::g_system_allocator}
{
}

inline constexpr string_table::string_table(memory::allocator* _allocator)
  : m_data{_allocator}
{
}

inline string_table::string_table(vector<char>&& data_)
  : m_data{utility::move(data_)}
{
}

inline string_table::string_table(string_table&& string_table_)
  : m_data{utility::move(string_table_.m_data)}
{
}

inline string_table::string_table(const string_table& _string_table)
  : m_data{_string_table.m_data}
{
}

inline string_table& string_table::operator=(string_table&& string_table_) {
  m_data = utility::move(string_table_.m_data);
  return *this;
}

inline string_table& string_table::operator=(const string_table& _string_table) {
  m_data = _string_table.m_data;
  return *this;
}

inline const char* string_table::operator[](rx_size _index) const {
  return &m_data[_index];
}

inline const char* string_table::data() const {
  return m_data.data();
}

inline rx_size string_table::size() const {
  return m_data.size();
}

inline void string_table::clear() {
  m_data.clear();
}

inline memory::allocator* string_table::allocator() const {
  return m_data.allocator();
}

} // namespace rx

#endif // RX_CORE_STRING_TABLE_H
