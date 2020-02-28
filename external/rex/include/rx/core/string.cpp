#include <string.h> // strcmp, memcpy, memmove
#include <stdarg.h> // va_{list, start, end, copy}
#include <stdio.h> // vsnprintf

#include "rx/core/string.h" // string

#include "rx/core/utility/swap.h"

#include "rx/core/hints/unreachable.h"

namespace rx {

static void format_va(string& contents_, const char* _format, va_list _va) {
  // calculate length to format
  va_list ap;
  va_copy(ap, _va);
  const int length{vsnprintf(nullptr, 0, _format, ap)};
  va_end(ap);

  // format into string
  contents_.resize(length);
  vsnprintf(contents_.data(), contents_.size() + 1, _format, _va);
}

rx_size utf8_to_utf16(const char* _utf8_contents, rx_size _length,
  rx_u16* utf16_contents_)
{
  rx_size elements{0};
  rx_u32 code_point{0};

  for (rx_size i{0}; i < _length; i++) {
    const char* element{_utf8_contents + i};
    const rx_byte element_ch{static_cast<rx_byte>(*element)};

    if (element_ch <= 0x7f) {
      code_point = static_cast<rx_u16>(element_ch);
    } else if (element_ch <= 0xbf) {
      code_point = (code_point << 6) | (element_ch & 0x3f);
    } else if (element_ch <= 0xdf) {
      code_point = element_ch & 0x1f;
    } else if (element_ch <= 0xef) {
      code_point = element_ch & 0x0f;
    } else {
      code_point = element_ch & 0x07;
    }

    element++;
    if ((*element & 0xc0) != 0x80 && code_point <= 0x10ffff) {
      if (code_point > 0xffff) {
        elements += 2;
        if (utf16_contents_) {
          *utf16_contents_++ = static_cast<rx_u16>(0xd800 + (code_point >> 10));
          *utf16_contents_++ = static_cast<rx_u16>(0xdc00 + (code_point & 0x03ff));
        }
      } else if (code_point < 0xd800 || code_point >= 0xe000) {
        elements += 1;
        if (utf16_contents_) {
          *utf16_contents_++ = static_cast<rx_u16>(code_point);
        }
      }
    }
  }

  return elements;
}

rx_size utf16_to_utf8(const rx_u16* _utf16_contents, rx_size _length,
  char* utf8_contents_)
{
  rx_size elements{0};
  rx_u32 code_point{0};

  for (rx_size i{0}; i < _length; i++) {
    const rx_u16* element{_utf16_contents+i};

    if (*element >= 0xd800 && *element <= 0xdbff) {
      code_point = ((*element - 0xd800) << 10) + 0x10000;
    } else {
      if (*element >= 0xdc00 && *element <= 0xdfff) {
        code_point |= *element - 0xdc00;
      } else {
        code_point = *element;
      }

      if (code_point < 0x7f) {
        elements += 1;
        if (utf8_contents_) {
          *utf8_contents_++ = static_cast<char>(code_point);
        }
      } else if (code_point <= 0x7ff) {
        elements += 2;
        if (utf8_contents_) {
          *utf8_contents_++ = static_cast<char>(0xc0 | ((code_point >> 6) & 0x1f));
          *utf8_contents_++ = static_cast<char>(0x80 | (code_point & 0x3f));
        }
      } else if (code_point <= 0xffff) {
        elements += 3;
        if (utf8_contents_) {
          *utf8_contents_++ = static_cast<char>(0xe0 | ((code_point >> 12) & 0x0f));
          *utf8_contents_++ = static_cast<char>(0x80 | ((code_point >> 6) & 0x3f));
          *utf8_contents_++ = static_cast<char>(0x80 | (code_point & 0x3f));
        }
      } else {
        elements += 4;
        if (utf8_contents_) {
          *utf8_contents_++ = static_cast<char>(0xf0 | ((code_point >> 18) & 0x07));
          *utf8_contents_++ = static_cast<char>(0x80 | ((code_point >> 12) & 0x3f));
          *utf8_contents_++ = static_cast<char>(0x80 | ((code_point >> 6) & 0x3f));
          *utf8_contents_++ = static_cast<char>(0x80 | (code_point & 0x3f));
        }
      }

      code_point = 0;
    }
  }

  return elements;
}

string string::formatter(memory::allocator* _allocator,
  const char* _format, ...)
{
  va_list va;
  va_start(va, _format);
  string contents{_allocator};
  format_va(contents, _format, va);
  va_end(va);
  return contents;
}

string::string(memory::allocator* _allocator, const char* _contents)
  : string{_allocator, _contents, strlen(_contents)}
{
}

string::string(memory::allocator* _allocator, const char* _contents,
  rx_size _size)
  : string{_allocator}
{
  append(_contents, _size);
}

string::string(memory::allocator* _allocator, const char* _first,
  const char* _last)
  : string{_allocator}
{
  append(_first, _last);
}

string::string(string&& contents_)
  : m_allocator{contents_.m_allocator}
{
  RX_ASSERT(m_allocator, "null allocator");

  if (contents_.m_data == contents_.m_buffer) {
    m_data = m_buffer;
    m_last = m_buffer;
    m_capacity = m_buffer + k_small_string;
    reserve(contents_.size());
    append(contents_.data(), contents_.size());
  } else {
    m_data = contents_.m_data;
    m_last = contents_.m_last;
    m_capacity = contents_.m_capacity;
  }

  contents_.m_data = contents_.m_buffer;
  contents_.m_last = contents_.m_buffer;
  contents_.m_capacity = contents_.m_buffer + k_small_string;
  contents_.resize(0);
}

string::string(memory::view _view)
  : m_allocator{_view.owner}
  , m_data{nullptr}
  , m_last{nullptr}
  , m_capacity{nullptr}
{
  // Search for the null-terminator in the view to find the end of the string.
  for (rx_size i{0}; i < _view.size; i++) {
    if (_view.data[i] == 0) {
      m_last = reinterpret_cast<char*>(_view.data + i);
      break;
    }
  }

  if (m_last) {
    m_data = reinterpret_cast<char*>(_view.data);
    m_capacity = reinterpret_cast<char*>(_view.data + _view.size);
  } else {
    // Could not find a terminator. Resize the memory given by the view, which
    // can potentially be done in place, append the null-terminator and fill
    // out the string.
    rx_byte* data = m_allocator->reallocate(_view.data, _view.size + 1);
    RX_ASSERT(data, "out of memory");

    data[_view.size] = 0;

    m_data = reinterpret_cast<char*>(data);
    m_last = m_data + _view.size;
    m_capacity = m_data + _view.size;
  }
}

string::~string() {
  if (m_data != m_buffer) {
    m_allocator->deallocate(reinterpret_cast<rx_byte*>(m_data));
  }
}

string& string::operator=(const string& _contents) {
  RX_ASSERT(&_contents != this, "self assignment");
  string(_contents).swap(*this);
  return *this;
}

string& string::operator=(const char* _contents) {
  RX_ASSERT(_contents, "empty string");
  string(_contents).swap(*this);
  return *this;
}

string& string::operator=(string&& contents_) {
  RX_ASSERT(&contents_ != this, "self assignment");
  string(utility::move(contents_)).swap(*this);
  return *this;
}

void string::reserve(rx_size _capacity) {
  if (m_data + _capacity + 1 <= m_capacity) {
    return;
  }

  char* data{nullptr};
  const auto size{static_cast<rx_size>(m_last - m_data)};
  if (m_data == m_buffer) {
    data = reinterpret_cast<char*>(m_allocator->allocate(_capacity + 1));
    if (size) {
      memcpy(data, m_data, size + 1);
    }
  } else {
    data = reinterpret_cast<char*>(m_allocator->reallocate(reinterpret_cast<rx_byte*>(m_data), _capacity + 1));
  }

  m_data = data;
  m_last = m_data + size;
  m_capacity = m_data + _capacity;
}

void string::resize(rx_size _size) {
  const auto previous_size{static_cast<rx_size>(m_last - m_data)};

  reserve(_size);

  if (_size > previous_size) {
    char* element{m_last};
    char* end{m_data + _size + 1};
    for (; element < end; ++element) {
      *element = '\0';
    }
  } else if (m_last != m_data) {
    m_data[_size] = '\0';
  }

  m_last = m_data + _size;
}

rx_size string::find_first_of(int _ch) const {
  if (char* search{strchr(m_data, _ch)}) {
    return search - m_data;
  }
  return k_npos;
}

rx_size string::find_first_of(const char* _contents) const {
  if (const char* search{strstr(m_data, _contents)}) {
    return search - m_data;
  }
  return k_npos;
}

rx_size string::find_last_of(int _ch) const {
  if (const char* search{strrchr(m_data, _ch)}) {
    return search - m_data;
  }
  return k_npos;
}

rx_size string::find_last_of(const char* _contents) const {
  auto reverse_strstr{[](const char* _haystack, const char* _needle) {
    const char* r{nullptr};
    for (;;) {
      const char* p = strstr(_haystack, _needle);
      if (!p) {
        return r;
      }
      r = p;
      _haystack = p + 1;
    }
  }};

  if (const char* search{reverse_strstr(m_data, _contents)}) {
    return search - m_data;
  }
  return k_npos;
}

string& string::append(const char* _first, const char *_last) {
  const auto new_size{static_cast<rx_size>(m_last - m_data + _last - _first + 1)};
  if (m_data + new_size > m_capacity) {
    reserve((new_size * 3) / 2);
  }

  const auto length{static_cast<rx_size>(_last - _first)};
  memcpy(m_last, _first, length);
  m_last += length;
  *m_last = '\0';

  return *this;
}

string& string::append(const char* _contents) {
  return append(_contents, strlen(_contents));
}

void string::insert_at(rx_size _position, const char* _contents, rx_size _size) {
  const rx_size old_this_size{size()};
  const rx_size old_that_size{_size};
  resize(old_this_size + old_that_size);
  char* cursor{m_data + _position};
  memmove(cursor + old_that_size, cursor, old_this_size - _position);
  memmove(cursor, _contents, old_that_size);
}

void string::insert_at(rx_size _position, const char* _contents) {
  insert_at(_position, _contents, strlen(_contents));
}

string string::lstrip(const char* _set) const {
  const char* ch{m_data};
  for (; strchr(_set, *ch); ch++);
  return {m_allocator, ch};
}

string string::rstrip(const char* _set) const {
  const char* ch{m_data + size()};
  for (; strchr(_set, *ch); ch--);
  return {m_allocator, m_data, ch};
}

vector<string> string::split(memory::allocator* _allocator, int _token, rx_size _count) const {
  bool quoted{false};
  bool limit{_count > 0};
  vector<string> result{_allocator};

  // When there is a limit we can reserve the storage upfront.
  if (limit) {
    result.reserve(_count);
  }

  result.push_back("");
  _count--;

  for (const char* ch{m_data}; *ch; ch++) {
    // Handle escapes of quoted strings.
    if (*ch == '\\' && (ch[1] == '\\' || ch[1] == '\"')) {
      if (ch[1] == '\\') {
        result.last() += "\\";
      }

      if (ch[1] == '\"') {
        result.last() += "\"";
      }

      ch += 2;
      continue;
    }

    // Handle quoted strings.
    if (_count && *ch == '\"') {
      quoted = !quoted;
      continue;
    }

    if (*ch == _token && !quoted && (!limit || _count)) {
      result.push_back("");
      _count--;
    } else {
      result.last() += *ch;
    }
  }

  return result;
}

string string::substring(rx_size _offset, rx_size _length) const {
  const char* begin{m_data + _offset};
  RX_ASSERT(begin < m_data + size(), "out of bounds");
  if (_length == 0) {
    return {m_allocator, begin};
  }
  // NOTE(dweiler): You can substring the whole string.
  RX_ASSERT(begin + _length <= m_data + size(), "out of bounds");
  return {m_allocator, begin, _length};
}

rx_size string::scan(const char* _scan_format, ...) const {
  va_list va;
  va_start(va, _scan_format);
  const auto result{vsscanf(m_data, _scan_format, va)};
  va_end(va);
  return static_cast<rx_size>(result);
}

char string::pop_back() {
  if (m_last == m_data) {
    return *data();
  }
  m_last--;
  const char last{static_cast<char>(*m_last)};
  *m_last = 0;
  return last;
}

void string::erase(rx_size _begin, rx_size _end) {
  RX_ASSERT(_begin < _end, "invalid range");
  RX_ASSERT(_begin < size(), "out of bounds");
  RX_ASSERT(_end <= size(), "out of bounds");

  char *const begin{m_data + _begin};
  char *const end{m_data + _end};

  const auto length{m_last - end};

  memmove(begin, end, length);
  m_last = begin + length;
  *m_last = '\0';
}

string string::human_size_format(rx_size _size) {
  static constexpr const char* k_suffixes[]{
    "B", "KiB", "MiB", "GiB", "TiB"
  };

  rx_f64 bytes{static_cast<rx_f64>(_size)};
  rx_size i{0};
  for (; bytes >= 1024.0 && i < sizeof k_suffixes / sizeof *k_suffixes; i++) {
    bytes /= 1024.0;
  }
  RX_ASSERT(i != sizeof k_suffixes / sizeof *k_suffixes, "out of bounds");

  char buffer[2*(DBL_MANT_DIG + DBL_MAX_EXP)];

  // NOTE: The truncation is wanted here!
#if defined(RX_COMPILER_GCC)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif
  const int result{snprintf(buffer, sizeof buffer, "%.*f",
    static_cast<int>(sizeof buffer), bytes)};
#if defined(RX_COMPILER_GCC)
#pragma GCC diagnostic pop
#endif

  RX_ASSERT(result > 0, "failed to format");
  char* period{strchr(buffer, '.')};
  RX_ASSERT(period, "failed to format");
  period[3] = '\0';
  return format("%s %s", buffer, k_suffixes[i]);
}

bool string::begins_with(const char* _prefix) const {
  return strstr(m_data, _prefix) == m_data;
}

bool string::begins_with(const string& _prefix) const {
  return strstr(m_data, _prefix.data()) == m_data;
}

bool string::ends_with(const char* _suffix) const {
  if (size() < strlen(_suffix)) {
    return false;
  }

  return strcmp(m_data + size() - strlen(_suffix), _suffix) == 0;
}

bool string::ends_with(const string& _suffix) const {
  if (size() < _suffix.size()) {
    return false;
  }

  return strcmp(m_data + size() - _suffix.size(), _suffix.data()) == 0;
}

bool string::contains(const char* _needle) const {
  return strstr(m_data, _needle);
}

bool string::contains(const string& _needle) const {
  return strstr(m_data, _needle.data());
}

rx_size string::hash() const {
  // The following is an implementation of FNV1a. The difference here matters
  // because rx_size is the hash type which may be 4-byte or 8-byte depending
  // on architecture.
  //
  // The previous hash function was just DJB2 X=33 which left most of the
  // rx_size empty on 64-bit for short keys.
  if constexpr (sizeof(rx_size) == 8) {
    static constexpr const rx_u64 k_prime = 0x100000001b3_u64;
    rx_u64 hash = 0xcbf29ce484222325_u64;
    for (const char *ch = m_data; *ch; ch++) {
      const rx_byte value = *ch;
      hash = hash ^ value;
      hash *= k_prime;
    }
    return static_cast<rx_size>(hash);
  } else {
    static constexpr const rx_u32 k_prime = 0x1000193_u32;
    rx_u32 hash = 0x811c9dc5_u32;
    for (const char *ch = m_data; *ch; ch++) {
      const rx_byte value = *ch;
      hash = hash ^ value;
      hash &= k_prime;
    }
    return static_cast<rx_size>(hash);
  }
  RX_HINT_UNREACHABLE();
}

memory::view string::disown() {
  rx_byte* data{reinterpret_cast<rx_byte*>(m_data)};
  rx_size n_bytes{0};

  if (m_data == m_buffer) {
    // Cannot disown the memory of small string optimization. Copy it out into
    // an owning view.
    n_bytes = size() + 1;
    data = m_allocator->allocate(n_bytes);
    RX_ASSERT(data, "out of memory");
    memcpy(data, m_data, n_bytes);
  } else {
    // The memory allocated by the string is potentially larger than the whole
    // string, allow the view to reflect the full capacity.
    n_bytes = capacity();
  }

  // Reset the string back to small string optimization state.
  m_data = m_buffer;
  m_last = m_buffer;
  m_capacity = m_buffer + k_small_string;

  return {m_allocator, data, n_bytes};
}

wide_string string::to_utf16() const {
  rx_size length{utf8_to_utf16(m_data, size(), nullptr)};
  wide_string contents{m_allocator};
  contents.resize(length);
  utf8_to_utf16(m_data, length, contents.data());
  return contents;
}

// Complicated because of small string optimization.
void string::swap(string& other_) {
  RX_ASSERT(&other_ != this, "self swap");

  utility::swap(m_data, other_.m_data);
  utility::swap(m_last, other_.m_last);
  utility::swap(m_capacity, other_.m_capacity);

  char buffer[k_small_string];

  if (m_data == other_.m_buffer) {
    const char* element{other_.m_buffer};
    const char* end{m_last};
    char* out{buffer};

    while (element != end) {
      *out++ = *element++;
    }
  }

  if (other_.m_data == m_buffer) {
    other_.m_last = other_.m_last - other_.m_data + other_.m_buffer;
    other_.m_data = other_.m_buffer;
    other_.m_capacity = other_.m_buffer + k_small_string;

    char* element{other_.m_data};
    const char* end{other_.m_last};
    const char* in{m_buffer};

    while (element != end) {
      *element++ = *in++;
    }

    *other_.m_last = '\0';
  }

  if (m_data == other_.m_buffer) {
    m_last = m_last - m_data + m_buffer;
    m_data = m_buffer;
    m_capacity = m_buffer + k_small_string;

    char* element{m_data};
    const char* end{m_last};
    const char* in{buffer};

    while (element != end) {
      *element++ = *in++;
    }

    *m_last = '\0';
  }
}

bool operator==(const string& _lhs, const string& _rhs) {
  if (&_lhs == &_rhs) {
    return true;
  }

  if (_lhs.size() != _rhs.size()) {
    return false;
  }

  return !strcmp(_lhs.data(), _rhs.data());
}

bool operator!=(const string& _lhs, const string& _rhs) {
  if (&_lhs == &_rhs) {
    return false;
  }

  if (_lhs.size() != _rhs.size()) {
    return true;
  }

  return strcmp(_lhs.data(), _rhs.data());
}

bool operator<(const string& _lhs, const string& _rhs) {
  return &_lhs == &_rhs ? false : strcmp(_lhs.data(), _rhs.data()) < 0;
}

bool operator>(const string& _lhs, const string& _rhs) {
  return &_lhs == &_rhs ? false : strcmp(_lhs.data(), _rhs.data()) > 0;
}

void wide_string::resize(rx_size _size) {
  m_data = reinterpret_cast<rx_u16*>(m_allocator->reallocate(reinterpret_cast<rx_byte*>(m_data), (_size + 1) * sizeof *m_data));
  m_data[_size] = 0;
  m_size = _size;
}

string wide_string::to_utf8() const {
  rx_size size{utf16_to_utf8(m_data, m_size, nullptr)};
  string contents{m_allocator};
  contents.resize(size);
  utf16_to_utf8(m_data, m_size, contents.data());
  return contents;
}

static rx_size utf16_len(const rx_u16* _data) {
  rx_size length{0};
  while (_data[length]) {
    ++length;
  }
  return length;
}

// wide_string
wide_string::wide_string(memory::allocator* _allocator)
  : m_allocator{_allocator}
  , m_data{nullptr}
  , m_size{0}
{
}

wide_string::wide_string(memory::allocator* _allocator, const wide_string& _other)
  : wide_string{_allocator, _other.data(), _other.size()}
{
}

wide_string::wide_string(memory::allocator* _allocator, const rx_u16* _contents)
  : wide_string{_allocator, _contents, utf16_len(_contents)}
{
}

wide_string::wide_string(memory::allocator* _allocator, const rx_u16* _contents,
  rx_size _size)
  : m_allocator{_allocator}
  , m_size{_size}
{
  RX_ASSERT(m_allocator, "null allocator");

  m_data = reinterpret_cast<rx_u16*>(m_allocator->allocate(sizeof(rx_u16) * (_size + 1)));
  RX_ASSERT(m_data, "out of memory");

  memcpy(m_data, _contents, sizeof(rx_u16) * (_size + 1));
}

wide_string::wide_string(wide_string&& other_)
  : m_allocator{other_.m_allocator}
  , m_data{other_.m_data}
  , m_size{other_.m_size}
{
  other_.m_data = nullptr;
  other_.m_size = 0;
}

wide_string::~wide_string() {
  m_allocator->deallocate(reinterpret_cast<rx_byte*>(m_data));
}

} // namespace rx
