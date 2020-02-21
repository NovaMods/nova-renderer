#ifndef RX_CORE_FILESYSTEM_FILE_H
#define RX_CORE_FILESYSTEM_FILE_H
#include "rx/core/stream.h"
#include "rx/core/string.h"

namespace rx::filesystem {

struct file
  final : stream
{
  constexpr file();
  constexpr file(memory::allocator* _allocator);
  file(memory::allocator* _allocator, const char* _file_name, const char* _mode);
  file(memory::allocator* _allocator, const string& _file_name, const char* _mode);
  file(const char* _file_name, const char* _mode);
  file(const string& _file_name, const char* _mode);
  file(file&& other_);
  ~file();

  file& operator=(file&& file_);

  // Read |_size| bytes from file into |_data|.
  virtual rx_u64 on_read(rx_byte* _data, rx_u64 _size);

  // Write |_size| bytes from |_data| into file.
  virtual rx_u64 on_write(const rx_byte* _data, rx_u64 _size);

  // Seek to |_where| in file relative to |_whence|.
  virtual bool on_seek(rx_s64 _where, whence _whence);

  // Tell where we're at in the file.
  virtual rx_u64 on_tell();

  // Flush to disk.
  virtual bool on_flush();

  bool read_line(string& line_);
  bool close();

  // Print |_fmt| with |_args| to file using |_allocator| for formatting.
  // NOTE: asserts if the file isn't a text file.
  template<typename... Ts>
  bool print(memory::allocator* _alloc, const char* _fmt, Ts&&... _args);

  // Print |_fmt| with |_args| to file using system allocator for formatting.
  // NOTE: asserts if the file isn't a text file.
  template<typename... Ts>
  bool print(const char* _fmt, Ts&&... _args);

  // Print a string into the file. This is only valid for text files.
  // NOTE: asserts if the file isn't a text file.
  bool print(string&& contents_);

  // Query if the file handle is valid, will be false if the file has been
  // closed with |close| or if the file failed to open.
  bool is_valid() const;

  operator bool() const;

  virtual const string& name() const &;

  memory::allocator* allocator() const;

private:
  file(void* _impl, const char* _file_name, const char* _mode);

  static rx_u32 flags_from_mode(const char* _mode);

  friend struct process;

  memory::allocator* m_allocator;
  void* m_impl;
  string m_name;
  const char* m_mode;
};

inline constexpr file::file()
  : file{&memory::g_system_allocator}
{
}

inline constexpr file::file(memory::allocator* _allocator)
  : stream{0}
  , m_allocator{_allocator}
  , m_impl{nullptr}
  , m_name{m_allocator}
  , m_mode{nullptr}
{
}

inline file::file(memory::allocator* _allocator, const string& _file_name, const char* _mode)
  : file{_allocator, _file_name.data(), _mode}
{
}

inline file::file(const char* _file_name, const char* _mode)
  : file{&memory::g_system_allocator, _file_name, _mode}
{
}

inline file::file(const string& _file_name, const char* _mode)
  : file{&memory::g_system_allocator, _file_name, _mode}
{
}

inline file::file(file&& other_)
  : stream{utility::move(other_)}
  , m_allocator{other_.m_allocator}
  , m_impl{other_.m_impl}
  , m_name{utility::move(other_.m_name)}
  , m_mode{other_.m_mode}
{
  other_.m_allocator = nullptr;
  other_.m_impl = nullptr;
  other_.m_mode = nullptr;
}

inline file::~file() {
  close();
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
optional<vector<rx_byte>> read_text_file(memory::allocator* _allocator, const char* _file_name);

inline optional<vector<rx_byte>> read_binary_file(memory::allocator* _allocator, const string& _file_name) {
  return read_binary_file(_allocator, _file_name.data());
}

inline optional<vector<rx_byte>> read_binary_file(const string& _file_name) {
  return read_binary_file(&memory::g_system_allocator, _file_name);
}

inline optional<vector<rx_byte>> read_binary_file(const char* _file_name) {
  return read_binary_file(&memory::g_system_allocator, _file_name);
}

inline optional<vector<rx_byte>> read_text_file(memory::allocator* _allocator, const string& _file_name) {
  return read_text_file(_allocator, _file_name.data());
}

inline optional<vector<rx_byte>> read_text_file(const string& _file_name) {
  return read_text_file(&memory::g_system_allocator, _file_name);
}

inline optional<vector<rx_byte>> read_text_file(const char* _file_name) {
  return read_text_file(&memory::g_system_allocator, _file_name);
}

} // namespace rx::filesystem

#endif // RX_CORE_FILESYSTEM_FILE_H
