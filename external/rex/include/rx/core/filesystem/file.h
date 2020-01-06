#ifndef RX_CORE_FILESYSTEM_FILE_H
#define RX_CORE_FILESYSTEM_FILE_H
#include "rx/core/string.h" // string
#include "rx/core/optional.h" // optional
#include "rx/core/concepts/no_copy.h" // no_copy

namespace rx::filesystem {

struct file
  : concepts::no_copy
{
  constexpr file();

  file(void* _impl, const char* _file_name, const char* _mode);
  file(const char* _file_name, const char* _mode);
  file(const string& _file_name, const char* _mode);
  file(file&& other_);
  ~file();

  file& operator=(file&& file_);

  // close file
  void close();

  // read |_size| bytes from file into |_data|
  rx_u64 read(rx_byte* _data, rx_u64 _size);

  // write |_size| bytes from |_data| into file
  rx_u64 write(const rx_byte* _data, rx_u64 _size);

  // print |fmt| with |args| to file using |alloc| for formatting
  template<typename... Ts>
  bool print(memory::allocator* _alloc, const char* _fmt, Ts&&... _args);

  // print |fmt| with |args| to file
  template<typename... Ts>
  bool print(const char* _fmt, Ts&&... _args);

  // seek to |where| in file
  bool seek(rx_u64 _where);

  // get size of file, returns nullopt if operation not supported
  optional<rx_u64> size();

  // flush to disk
  bool flush();

  bool read_line(string& line_);

  bool is_valid() const;

  operator bool() const;

  bool print(string&& contents_);

private:
  friend struct process;

  void* m_impl;

  const char* m_file_name;
  const char* m_mode;
};

inline constexpr file::file()
  : m_impl{nullptr}
  , m_file_name{nullptr}
  , m_mode{nullptr}
{
}

inline file::file(const string& _file_name, const char* _mode)
  : file{_file_name.data(), _mode}
{
}

inline file::operator bool() const {
  return is_valid();
}

template<typename... Ts>
inline bool file::print(memory::allocator* _allocator, const char* _format, Ts&&... _arguments) {
  return print(string::format(_allocator, _format, utility::forward<Ts>(_arguments)...));
}

template<typename... Ts>
inline bool file::print(const char* _format, Ts&&... _arguments) {
  return print(&memory::g_system_allocator, _format, utility::forward<Ts>(_arguments)...);
}

optional<vector<rx_byte>> read_binary_file(memory::allocator* _allocator, const char* _file_name);

inline optional<vector<rx_byte>> read_binary_file(memory::allocator* _allocator, const string& _file_name) {
  return read_binary_file(_allocator, _file_name.data());
}

inline optional<vector<rx_byte>> read_binary_file(const string& _file_name) {
  return read_binary_file(&memory::g_system_allocator, _file_name);
}

inline optional<vector<rx_byte>> read_binary_file(const char* _file_name) {
  return read_binary_file(&memory::g_system_allocator, _file_name);
}

} // namespace rx::filesystem

#endif // RX_CORE_FILESYSTEM_FILE_H
