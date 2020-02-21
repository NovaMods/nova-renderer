#ifndef RX_CORE_STRING_H
#define RX_CORE_STRING_H
#include "rx/core/assert.h" // RX_ASSERT
#include "rx/core/format.h" // format
#include "rx/core/vector.h" // vector

#include "rx/core/traits/remove_cvref.h"

#include "rx/core/memory/system_allocator.h" // memory::{system_allocator, allocator}

namespace rx {

struct wide_string;

// 32-bit: 16 + k_small_string bytes
// 64-bit: 32 + k_small_string bytes
struct string {
  static constexpr const rx_size k_npos{-1_z};
  static constexpr const rx_size k_small_string{16};

  constexpr string(memory::allocator* _allocator);
  string(memory::allocator* _allocator, const string& _contents);
  string(memory::allocator* _allocator, const char* _contents);
  string(memory::allocator* _allocator, const char* _contents, rx_size _size);
  string(memory::allocator* _allocator, const char* _first, const char* _last);

  constexpr string();
  string(const string& _contents);
  string(string&& contents_);
  string(const char* _contents);
  string(const char* _contents, rx_size _size);
  string(const char* _first, const char* _last);
  string(memory::view _view);
  ~string();

  template<typename... Ts>
  static string format(memory::allocator* _allocator,
    const char* _format, Ts&&... _arguments);

  template<typename... Ts>
  static string format(const char* _format, Ts&&... _arguments);

  string& operator=(const string& _contents);
  string& operator=(const char* _contents);
  string& operator=(string&& contents_);

  void reserve(rx_size _size);
  void resize(rx_size _size);

  rx_size find_first_of(int _ch) const;
  rx_size find_first_of(const char* _contents) const;
  rx_size find_first_of(const string& _contents) const;

  rx_size find_last_of(int _ch) const;
  rx_size find_last_of(const char* _contents) const;
  rx_size find_last_of(const string& _contents) const;

  rx_size size() const;
  rx_size capacity() const;

  bool is_empty() const;

  void clear();

  string& append(const char* _first, const char* _last);
  string& append(const char* _contents, rx_size _size);
  string& append(const char* _contents);
  string& append(const string& _contents);
  string& append(char _ch);

  void insert_at(rx_size _position, const char* _contents, rx_size _size);
  void insert_at(rx_size _position, const char* _contents);
  void insert_at(rx_size _position, const string& _contents);

  // returns copy of string with leading characters in set removed
  string lstrip(const char* _set) const;

  // returns copy of string with trailing characters in set removed
  string rstrip(const char* _set) const;

  // split string by |token| up to |count| times, use |count| of zero for no limit
  vector<string> split(memory::allocator* _allocator, int _ch, rx_size _count = 0) const;
  vector<string> split(int _ch, rx_size _count = 0) const;

  // take substring from |offset| of |length|, use |length| of zero for whole string
  string substring(rx_size _offset, rx_size _length = 0) const;

  // scan string
  rx_size scan(const char* _scan_format, ...) const;

  char pop_back();

  void erase(rx_size _begin, rx_size _end);

  char& operator[](rx_size _index);
  const char& operator[](rx_size _index) const;

  char& first();
  const char& first() const;

  char& last();
  const char& last() const;

  char* data();
  const char* data() const;

  static string human_size_format(rx_size _size);

  bool begins_with(const char* _prefix) const;
  bool begins_with(const string& _prefix) const;
  bool ends_with(const char* _suffix) const;
  bool ends_with(const string& _suffix) const;
  bool contains(const char* _needle) const;
  bool contains(const string& _needle) const;

  rx_size hash() const;

  wide_string to_utf16() const;

  memory::allocator* allocator() const;
  memory::view disown();

private:
  static string formatter(memory::allocator* _allocator, const char* _format, ...);

  void swap(string& other);

  memory::allocator* m_allocator;
  char* m_data;
  char* m_last;
  char* m_capacity;
  char m_buffer[k_small_string];
};

// utf-16, Windows compatible "wide-string"
struct wide_string {
  // custom allocator versions
  wide_string(memory::allocator* _allocator);
  wide_string(memory::allocator* _allocator, const wide_string& _other);
  wide_string(memory::allocator* _allocator, const rx_u16* _contents);
  wide_string(memory::allocator* _allocator, const rx_u16* _contents, rx_size _size);

  // constructors that use system allocator
  wide_string();
  wide_string(const wide_string& _other);
  wide_string(const rx_u16* _contents);
  wide_string(const rx_u16* _contents, rx_size _size);

  wide_string(wide_string&& other_);

  ~wide_string();

  // disable all assignment operators because you're not supposed to use wide_string
  // for any other purpose than to convert string (which is utf8) to utf16 for
  // interfaces expecting that, such as the ones on Windows
  wide_string& operator=(const wide_string&) = delete;
  wide_string& operator=(const rx_u16*) = delete;
  wide_string& operator=(const char*) = delete;
  wide_string& operator=(const string&) = delete;

  rx_size size() const;
  bool is_empty() const;

  rx_u16& operator[](rx_size _index);
  const rx_u16& operator[](rx_size _index) const;

  rx_u16* data();
  const rx_u16* data() const;

  void resize(rx_size _size);

  string to_utf8() const;

  memory::allocator* allocator() const;

private:
  memory::allocator* m_allocator;

  rx_u16* m_data;
  rx_size m_size;
};

// format function for string
template<>
struct format_type<string> {
  const char* operator()(const string& _value) const {
    return _value.data();
  }
};

template<typename... Ts>
inline string string::format(memory::allocator* _allocator, const char* _format, Ts&&... _arguments) {
  return formatter(_allocator, _format, format_type<traits::remove_cvref<Ts>>{}(utility::forward<Ts>(_arguments))...);
}

template<typename... Ts>
inline string string::format(const char* _format, Ts&&... _arguments) {
  return format(&memory::g_system_allocator, _format, utility::forward<Ts>(_arguments)...);
}

inline string::string(memory::allocator* _allocator, const string& _contents)
  : string{_allocator, _contents.data(), _contents.size()}
{
}

inline constexpr string::string(memory::allocator* _allocator)
  : m_allocator{_allocator}
  , m_data{m_buffer}
  , m_last{m_buffer}
  , m_capacity{m_buffer + k_small_string}
  , m_buffer{}
{
  RX_ASSERT(m_allocator, "null allocator");
  m_buffer[0] = '\0';
}

inline constexpr string::string()
  : string{&memory::g_system_allocator}
{
}

inline string::string(const string& _contents)
  : string{_contents.m_allocator, _contents}
{
}

inline string::string(const char* _contents)
  : string{&memory::g_system_allocator, _contents}
{
}

inline string::string(const char* _contents, rx_size _size)
  : string{&memory::g_system_allocator, _contents, _size}
{
}

inline string::string(const char* _first, const char* _last)
  : string{&memory::g_system_allocator, _first, _last}
{
}

inline rx_size string::find_first_of(const string& _contents) const {
  return find_first_of(_contents.data());
}

inline rx_size string::find_last_of(const string& _contents) const {
  return find_last_of(_contents.data());
}

RX_HINT_FORCE_INLINE rx_size string::size() const {
  return m_last - m_data;
}

RX_HINT_FORCE_INLINE rx_size string::capacity() const {
  return m_capacity - m_data;
}

RX_HINT_FORCE_INLINE bool string::is_empty() const {
  return m_last - m_data == 0;
}

inline void string::clear() {
  resize(0);
}

inline string& string::append(const char* contents, rx_size size) {
  return append(contents, contents + size);
}

inline string& string::append(const string& contents) {
  return append(contents.data(), contents.size());
}

inline string& string::append(char ch) {
  return append(&ch, 1);
}

inline void string::insert_at(rx_size _position, const string& _contents) {
  insert_at(_position, _contents.data(), _contents.size());
}

inline vector<string> string::split(int _ch, rx_size _count) const {
  return split(m_allocator, _ch, _count);
}

inline char& string::operator[](rx_size index) {
  // NOTE(dweiler): The <= is not a bug, indexing the null-terminator is allowed.
  RX_ASSERT(index <= size(), "out of bounds");
  return m_data[index];
}

inline const char& string::operator[](rx_size index) const {
  // NOTE(dweiler): The <= is not a bug, indexing the null-terminator is allowed.
  RX_ASSERT(index <= size(), "out of bounds");
  return m_data[index];
}

RX_HINT_FORCE_INLINE char& string::first() {
  return m_data[0];
}

RX_HINT_FORCE_INLINE const char& string::first() const {
  return m_data[0];
}

RX_HINT_FORCE_INLINE char& string::last() {
  return m_data[size() - 1];
}

RX_HINT_FORCE_INLINE const char& string::last() const {
  return m_data[size() - 1];
}

RX_HINT_FORCE_INLINE char* string::data() {
  return m_data;
}

RX_HINT_FORCE_INLINE const char* string::data() const {
  return m_data;
}

inline string operator+(const string& _lhs, const char* _rhs) {
  return string(_lhs).append(_rhs);
}

inline string operator+(const string& _lhs, const string& _rhs) {
  return string(_lhs).append(_rhs);
}

inline string operator+(const string& _lhs, const char _ch) {
  return string(_lhs).append(_ch);
}

inline string& operator+=(string& lhs_, const char* rhs) {
  lhs_.append(rhs);
  return lhs_;
}

inline string& operator+=(string& lhs_, char ch) {
  lhs_.append(ch);
  return lhs_;
}

inline string& operator+=(string& lhs_, const string& _contents) {
  lhs_.append(_contents);
  return lhs_;
}

// not inlined since it would explode code size
bool operator==(const string& lhs, const string& rhs);
bool operator!=(const string& lhs, const string& rhs);
bool operator<(const string& lhs, const string& rhs);
bool operator>(const string& lhs, const string& rhs);

RX_HINT_FORCE_INLINE memory::allocator* string::allocator() const {
  return m_allocator;
}

// wide_string
inline wide_string::wide_string()
  : wide_string{&memory::g_system_allocator}
{
}

inline wide_string::wide_string(const rx_u16* _contents)
  : wide_string{&memory::g_system_allocator, _contents}
{
}

inline wide_string::wide_string(const rx_u16* _contents, rx_size _size)
  : wide_string{&memory::g_system_allocator, _contents, _size}
{
}

inline wide_string::wide_string(const wide_string& _other)
  : wide_string{_other.m_allocator, _other}
{
}

RX_HINT_FORCE_INLINE rx_size wide_string::size() const {
  return m_size;
}

RX_HINT_FORCE_INLINE bool wide_string::is_empty() const {
  return m_size == 0;
}

inline rx_u16& wide_string::operator[](rx_size _index) {
  RX_ASSERT(_index <= m_size, "out of bounds");
  return m_data[_index];
}

inline const rx_u16& wide_string::operator[](rx_size _index) const {
  RX_ASSERT(_index <= m_size, "out of bounds");
  return m_data[_index];
}

RX_HINT_FORCE_INLINE rx_u16* wide_string::data() {
  return m_data;
}

RX_HINT_FORCE_INLINE const rx_u16* wide_string::data() const {
  return m_data;
}

RX_HINT_FORCE_INLINE memory::allocator* wide_string::allocator() const {
  return m_allocator;
}

rx_size utf16_to_utf8(const rx_u16* _utf16_contents, rx_size _length,
  char* utf8_contents_);

rx_size utf8_to_utf16(const char* _utf8_contents, rx_size _length,
  rx_u16* utf16_contents_);

RX_HINT_FORCE_INLINE rx::string operator""_s(const char* _contents, rx_size _length) {
  return {_contents, _length};
}

} // namespace rx

#endif // RX_CORE_STRING_H
