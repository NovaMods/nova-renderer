#ifndef RX_CORE_FILESYSTEM_DIRECTORY_H
#define RX_CORE_FILESYSTEM_DIRECTORY_H
#include "rx/core/string.h"
#include "rx/core/function.h"
#include "rx/core/concepts/no_copy.h"

namespace rx::filesystem {

struct directory
  : concepts::no_copy
{
  directory(memory::allocator* _allocator, const char* _path);
  directory(memory::allocator* _allocator, const string& _path);
  directory(const char* _path);
  directory(const string& _path);
  ~directory();

  operator bool() const;

  struct item {
    enum class type : rx_u8 {
      k_file,
      k_directory
    };

    bool is_file() const;
    bool is_directory() const;
    const string& name() const;
    string&& name();

  private:
    friend struct directory;

    item(string&& name_, type _type);

    string m_name;
    type m_type;
  };

  // enumerate directory with |_function| being called for each item
  // NOTE: does not consider hidden files, symbolic links, block devices, or ..
  void each(function<void(item&&)>&& _function);

  memory::allocator* allocator() const;

private:
  memory::allocator* m_allocator;
  void* m_impl;
};

inline directory::directory(memory::allocator* _allocator, const string& _path)
  : directory{_allocator, _path.data()}
{
}

inline directory::directory(const char* _path)
  : directory{&memory::g_system_allocator, _path}
{
}

inline directory::directory(const string& _path)
  : directory{&memory::g_system_allocator, _path}
{
}

inline directory::operator bool() const {
  return m_impl;
}

inline memory::allocator* directory::allocator() const {
  return m_allocator;
}

inline bool directory::item::is_file() const {
  return m_type == type::k_file;
}

inline bool directory::item::is_directory() const {
  return m_type == type::k_directory;
}

inline const string& directory::item::name() const {
  return m_name;
}

inline string&& directory::item::name() {
  return utility::move(m_name);
}

inline directory::item::item(string&& name_, type _type)
  : m_name{utility::move(name_)}
  , m_type{_type}
{
}

} // namespace rx::filesystem

#endif // RX_CORE_FILESYSTEM_DIRECTORY_H
