#ifndef RX_CORE_JSON_H
#define RX_CORE_JSON_H
#include "rx/core/concurrency/atomic.h"

#include "rx/core/traits/return_type.h"
#include "rx/core/traits/is_same.h"
#include "rx/core/traits/detect.h"

#include "rx/core/string.h"
#include "rx/core/optional.h"

#include "rx/core/utility/declval.h"

#include "lib/json.h"

namespace rx {

struct json {
  constexpr json();
  json(memory::allocator* _allocator, const char* _contents, rx_size _length);
  json(memory::allocator* _allocator, const char* _contents);
  json(memory::allocator* _allocator, const string& _contents);
  json(const char* _contents, rx_size _length);
  json(const char* _contents);
  json(const string& _contents);
  json(const json& _json);
  json(json&& json_);
  ~json();

  json& operator=(const json& _json);
  json& operator=(json&& json_);

  enum class type {
    k_array,
    k_boolean,
    k_null,
    k_number,
    k_object,
    k_string,
    k_integer
  };

  operator bool() const;
  optional<string> error() const;

  bool is_type(type _type) const;

  bool is_array() const;
  bool is_array_of(type _type) const;
  bool is_array_of(type _type, rx_size _size) const;
  bool is_boolean() const;
  bool is_null() const;
  bool is_number() const;
  bool is_object() const;
  bool is_string() const;
  bool is_integer() const;

  json operator[](rx_size _index) const;
  bool as_boolean() const;
  rx_f64 as_number() const;
  rx_f32 as_float() const;
  rx_s32 as_integer() const;
  json operator[](const char* _name) const;
  string as_string() const;
  string as_string_with_allocator(memory::allocator* _allocator) const;

  template<typename T>
  T decode(const T& _default) const;

  // # of elements for objects and arrays only
  rx_size size() const;
  bool is_empty() const;

  template<typename F>
  bool each(F&& _function) const;

  memory::allocator* allocator() const;

private:
  template<typename T>
  using has_from_json =
    decltype(utility::declval<T>().from_json(utility::declval<json>()));

  struct shared {
    shared(memory::allocator* _allocator, const char* _contents, rx_size _length);
    ~shared();

    shared* acquire();
    void release();

    memory::allocator* m_allocator;
    struct json_parse_result_s m_error;
    struct json_value_s* m_root;
    concurrency::atomic<rx_size> m_count;
  };

  json(shared* _shared, struct json_value_s* _head);

  shared* m_shared;
  struct json_value_s* m_value;
};

inline constexpr json::json()
  : m_shared{nullptr}
  , m_value{nullptr}
{
}

inline json::json(memory::allocator* _allocator, const string& _contents)
  : json{_allocator, _contents.data(), _contents.size()}
{
}

inline json::json(const char* _contents, rx_size _length)
  : json{&memory::g_system_allocator, _contents, _length}
{
}

inline json::json(const string& _contents)
  : json{&memory::g_system_allocator, _contents.data(), _contents.size()}
{
}

inline json::json(const json& _json)
  : m_shared{_json.m_shared->acquire()}
  , m_value{_json.m_value}
{
}

inline json::json(json&& json_)
  : m_shared{json_.m_shared}
  , m_value{json_.m_value}
{
  json_.m_shared = nullptr;
  json_.m_value = nullptr;
}

inline json::~json() {
  if (m_shared) {
    m_shared->release();
  }
}

inline json& json::operator=(const json& _json) {
  RX_ASSERT(&_json != this, "self assignment");

  if (m_shared) {
    m_shared->release();
  }

  m_shared = _json.m_shared->acquire();
  m_value = _json.m_value;

  return *this;
}

inline json& json::operator=(json&& json_) {
  RX_ASSERT(&json_ != this, "self assignment");

  m_shared = json_.m_shared;
  m_value = json_.m_value;
  json_.m_shared = nullptr;
  json_.m_value = nullptr;

  return *this;
}

inline json::operator bool() const {
  return m_shared && m_shared->m_root;
}

inline bool json::is_array() const {
  return is_type(type::k_array);
}

inline bool json::is_array_of(type _type) const {
  if (!is_array()) {
    return false;
  }

  return each([_type](const json& _value) {
    return _value.is_type(_type);
  });
}

inline bool json::is_array_of(type _type, rx_size _size) const {
  if (!is_array()) {
    return false;
  }

  if (size() != _size) {
    return false;
  }

  return each([_type](const json& _value) {
    return _value.is_type(_type);
  });
}

inline bool json::is_boolean() const {
  return is_type(type::k_boolean);
}

inline bool json::is_null() const {
  return is_type(type::k_null);
}

inline bool json::is_number() const {
  return is_type(type::k_number);
}

inline bool json::is_object() const {
  return is_type(type::k_object);
}

inline bool json::is_string() const {
  return is_type(type::k_string);
}

inline bool json::is_integer() const {
  return is_type(type::k_integer);
}

inline bool json::is_empty() const {
  return size() == 0;
}

inline string json::as_string() const {
  return as_string_with_allocator(&memory::g_system_allocator);
}

template<typename T>
inline T json::decode(const T& _default) const {
  if constexpr(traits::is_same<T, rx_f32> || traits::is_same<T, rx_f64>) {
    if (is_number()) {
      return as_number();
    }
  } else if constexpr(traits::is_same<T, rx_s32>) {
    if (is_integer()) {
      return as_integer();
    }
  } else if constexpr(traits::is_same<T, string>) {
    if (is_string()) {
      return as_string();
    }
  } else if constexpr(traits::detect<T, has_from_json>) {
    return T::from_json(*this);
  }

  return _default;
}

template<typename F>
inline bool json::each(F&& _function) const {
  const bool array = is_array();
  const bool object = is_object();

  RX_ASSERT(array || object, "not enumerable");

  if (array) {
    auto array = reinterpret_cast<struct json_array_s*>(m_value->payload);
    for (auto element = array->start; element; element = element->next) {
      if constexpr(traits::is_same<traits::return_type<F>, bool>) {
        if (!_function({m_shared, element->value})) {
          return false;
        }
      } else {
        _function({m_shared, element->value});
      }
    }
  } else {
    auto object = reinterpret_cast<struct json_object_s*>(m_value->payload);
    for (auto element = object->start; element; element = element->next) {
      if constexpr(traits::is_same<traits::return_type<F>, bool>) {
        if (!_function({m_shared, element->value})) {
          return false;
        }
      } else {
        _function({m_shared, element->value});
      }
    }
  }

  return true;
}

inline memory::allocator* json::allocator() const {
  return m_shared ? m_shared->m_allocator : nullptr;
}

} // namespace rx

#endif // RX_CORE_JSON_H
