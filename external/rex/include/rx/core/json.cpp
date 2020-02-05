#include <stdlib.h> // strtod
#include <string.h> // strcmp

#include "rx/core/json.h"
#include "rx/core/hints/unreachable.h"

#include "rx/core/math/floor.h"

namespace rx {

static const char* json_parse_error_to_string(enum json_parse_error_e _error) {
  switch (_error) {
  case json_parse_error_expected_comma_or_closing_bracket:
    return "expected either a comma or closing '}' or ']'";
  case json_parse_error_expected_colon:
    return "expected a colon";
  case json_parse_error_expected_opening_quote:
    return "expected opening quote '\"'";
  case json_parse_error_invalid_string_escape_sequence:
    return "invalid string escape sequence";
  case json_parse_error_invalid_number_format:
    return "invalid number formatting";
  case json_parse_error_invalid_value:
    return "invalid value";
  case json_parse_error_premature_end_of_buffer:
    return "premature end of buffer";
  case json_parse_error_invalid_string:
    return "malformed string";
  case json_parse_error_allocator_failed:
    return "out of memory";
  case json_parse_error_unexpected_trailing_characters:
    return "unexpected trailing characters";
  case json_parse_error_unknown:
    [[fallthrough]];
  default:
    return "unknown error";
  }
}

static void* json_allocator(void* _user, rx_size _size) {
  auto allocator{reinterpret_cast<memory::allocator*>(_user)};
  return allocator->allocate(_size);
}

json::shared::shared(memory::allocator* _allocator, const char* _contents, rx_size _length)
  : m_allocator{_allocator}
  , m_root{nullptr}
  , m_count{1}
{
  RX_ASSERT(m_allocator, "null allocator");

  m_root = json_parse_ex(_contents, _length,
    (json_parse_flags_allow_c_style_comments |
     json_parse_flags_allow_location_information |
     json_parse_flags_allow_unquoted_keys |
     json_parse_flags_allow_multi_line_strings),
    json_allocator,
    m_allocator,
    &m_error);
}

json::shared* json::shared::acquire() {
  RX_ASSERT(m_count, "consistency check failed");
  m_count++;
  return this;
}

void json::shared::release() {
  if (--m_count == 0) {
    m_allocator->destroy<shared>(this);
  }
}

json::shared::~shared() {
  m_allocator->deallocate(reinterpret_cast<rx_byte*>(m_root));
}

json::json(memory::allocator* _allocator, const char* _contents, rx_size _length)
  : m_shared{nullptr}
  , m_value{nullptr}
{
  RX_ASSERT(_allocator, "null allocator");

  // Construct the shared state.
  m_shared = _allocator->create<shared>(_allocator, _contents, _length);

  // We hold a reference to the shared state already. Just take the root JSON
  // value as the base to begin.
  m_value = m_shared->m_root;
}

json::json(memory::allocator* _allocator, const char* _contents)
  : json{_allocator, _contents, strlen(_contents)}
{
}

json::json(const char* _contents)
  : json{&memory::g_system_allocator, _contents, strlen(_contents)}
{
}

json::json(shared* _shared, struct json_value_s* _value)
  : m_shared{_shared->acquire()}
  , m_value{_value}
{
}

optional<string> json::error() const {
  if (m_shared) {
    const auto code{static_cast<enum json_parse_error_e>(m_shared->m_error.error)};
    return string::format("%zu:%zu %s", m_shared->m_error.error_line_no,
      m_shared->m_error.error_row_no, json_parse_error_to_string(code));
  }
  return nullopt;
}

bool json::is_type(type _type) const {
  switch (_type) {
  case type::k_array:
    return m_value->type == json_type_array;
  case type::k_boolean:
    return m_value->type == json_type_true || m_value->type == json_type_false;
  case type::k_integer:
    return m_value->type == json_type_number && math::floor(as_number()) == as_number();
  case type::k_null:
    return !m_value || m_value->type == json_type_null;
  case type::k_number:
    return m_value->type == json_type_number;
  case type::k_object:
    return m_value->type == json_type_object;
  case type::k_string:
    return m_value->type == json_type_string;
  }

  RX_HINT_UNREACHABLE();
}

json json::operator[](rx_size _index) const {
  RX_ASSERT(is_array() || is_object(), "not an indexable type");

  if (is_array()) {
    auto array{reinterpret_cast<struct json_array_s*>(m_value->payload)};
    auto element{array->start};
    RX_ASSERT(_index < array->length, "out of bounds");
    for (rx_size i{0}; i < _index; i++) {
      element = element->next;
    }
    return {m_shared, element->value};
  } else {
    auto object{reinterpret_cast<struct json_object_s*>(m_value->payload)};
    auto element{object->start};
    RX_ASSERT(_index < object->length, "out of bounds");
    for (rx_size i{0}; i < _index; i++) {
      element = element->next;
    }
    return {m_shared, element->value};
  }

  return {};
}

bool json::as_boolean() const {
  RX_ASSERT(is_boolean(), "not a boolean");
  return m_value->type == json_type_true;
}

rx_f64 json::as_number() const {
  RX_ASSERT(is_number(), "not a number");
  auto number{reinterpret_cast<struct json_number_s*>(m_value->payload)};
  return strtod(number->number, nullptr);
}

rx_f32 json::as_float() const {
  return static_cast<rx_f32>(as_number());
}

rx_s32 json::as_integer() const {
  RX_ASSERT(is_integer(), "not a integer");
  return static_cast<rx_s32>(as_number());
}

json json::operator[](const char* _name) const {
  RX_ASSERT(is_object(), "not a object");
  auto object{reinterpret_cast<struct json_object_s*>(m_value->payload)};
  for (auto element{object->start}; element; element = element->next) {
    if (!strcmp(element->name->string, _name)) {
      return {m_shared, element->value};
    }
  }
  return {};
}

string json::as_string_with_allocator(memory::allocator* _allocator) const {
  RX_ASSERT(is_string(), "not a string");
  RX_ASSERT(_allocator, "null allocator");
  auto string{reinterpret_cast<struct json_string_s*>(m_value->payload)};
  return {_allocator, string->string, string->string_size};
}

rx_size json::size() const {
  RX_ASSERT(is_array() || is_object(), "not an indexable type");
  switch (m_value->type) {
  case json_type_array:
    return reinterpret_cast<struct json_array_s*>(m_value->payload)->length;
  case json_type_object:
    return reinterpret_cast<struct json_object_s*>(m_value->payload)->length;
  }
  return 0;
}

} // namespace rx
