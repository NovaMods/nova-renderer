#include <stdio.h> // FILE, f{open,close,read,write,seek,flush}
#include <errno.h> // errno
#include <string.h> // strcmp, strerror

#include "rx/core/log.h"
#include "rx/core/assert.h"
#include "rx/core/config.h"

#include "rx/core/filesystem/file.h"

#include "rx/core/hints/unlikely.h"
#include "rx/core/hints/unreachable.h"

RX_LOG("filesystem/file", logger);

namespace rx::filesystem {

rx_u32 file::flags_from_mode(const char* _mode) {
  rx_u32 flags = 0;

  // These few flags are true regardless of the mode.
  flags |= stream::k_tell;
  flags |= stream::k_seek;

  for (const char* ch = _mode; *ch; ch++) {
    switch (*ch) {
    case 'r':
      flags |= stream::k_read;
      break;
    case 'w':
      [[fallthrough]];
    case '+':
      flags |= stream::k_write;
      flags |= stream::k_flush;
      break;
    }
  }

  return flags;
}

#if defined(RX_PLATFORM_WINDOWS)
file::file(memory::allocator* _allocator, const char* _file_name, const char* _mode)
  : stream{flags_from_mode(_mode)}
  , m_allocator{_allocator}
  , m_impl{nullptr}
  , m_name{m_allocator, _file_name}
  , m_mode{_mode}
{
  // Convert |_file_name| to UTF-16.
  const wide_string file_name = string(_file_name).to_utf16();

  // Convert the mode string to a wide char version. The mode string is in ascii
  // so there's no conversion necessary other than extending the type size.
  wchar_t mode_buffer[8];
  wchar_t *mode = mode_buffer;
  for (const char* ch = _mode; *ch; ch++) {
    *mode++ = static_cast<wchar_t>(*ch);
  }
  // Null-terminate mode.
  *mode++ = L'\0';

  // Utilize _wfopen on Windows so we can open files with UNICODE names.
  m_impl = static_cast<void*>(
    _wfopen(reinterpret_cast<const wchar_t*>(file_name.data()), mode_buffer));
}
#else
file::file(memory::allocator* _allocator, const char* _file_name, const char* _mode)
  : stream{flags_from_mode(_mode)}
  , m_allocator{_allocator}
  , m_impl{static_cast<void*>(fopen(_file_name, _mode))}
  , m_name{m_allocator, _file_name}
  , m_mode{_mode}
{
}
#endif

rx_u64 file::on_read(rx_byte* _data, rx_u64 _size) {
  RX_ASSERT(m_impl, "invalid");
  return fread(_data, 1, _size, static_cast<FILE*>(m_impl));
}

rx_u64 file::on_write(const rx_byte* _data, rx_u64 _size) {
  RX_ASSERT(m_impl, "invalid");
  return fwrite(_data, 1, _size, static_cast<FILE*>(m_impl));
}

bool file::on_seek(rx_s64 _where, whence _whence) {
  RX_ASSERT(m_impl, "invalid");

  const auto fp = static_cast<FILE*>(m_impl);
  const auto where = static_cast<long>(_where);

  switch (_whence) {
  case whence::k_set:
    return fseek(fp, where, SEEK_SET) == 0;
  case whence::k_current:
    return fseek(fp, where, SEEK_CUR) == 0;
  case whence::k_end:
    return fseek(fp, where, SEEK_END) == 0;
  }

  RX_HINT_UNREACHABLE();
}

rx_u64 file::on_tell() {
  RX_ASSERT(m_impl, "invalid");
  const auto fp = static_cast<FILE*>(m_impl);
  return ftell(fp);
}

bool file::on_flush() {
  RX_ASSERT(m_impl, "invalid");
  return fflush(static_cast<FILE*>(m_impl)) == 0;
}

bool file::close() {
  if (m_impl) {
    fclose(static_cast<FILE*>(m_impl));
    m_impl = nullptr;
    return true;
  }
  return false;
}

file& file::operator=(file&& file_) {
  RX_ASSERT(&file_ != this, "self assignment");

  close();
  m_impl = file_.m_impl;
  file_.m_impl = nullptr;
  return *this;
}

bool file::print(string&& contents_) {
  RX_ASSERT(m_impl, "invalid");
  RX_ASSERT(strcmp(m_mode, "w") == 0, "cannot print with mode '%s'", m_mode);
  return fprintf(static_cast<FILE*>(m_impl), "%s", contents_.data()) > 0;
}

bool file::read_line(string& line_) {
  auto* fp = static_cast<FILE*>(m_impl);

  line_.clear();
  for (;;) {
    char buffer[4096];
    if (!fgets(buffer, sizeof buffer, fp)) {
      if (feof(fp)) {
        return !line_.is_empty();
      }

      return false;
    }

    rx_size length{strlen(buffer)};

    if (length && buffer[length - 1] == '\n') {
      length--;
    }

    if (length && buffer[length - 1] == '\r') {
      length--;
    }

    line_.append(buffer, length);

    if (length < sizeof buffer - 1) {
      return true;
    }
  }

  return false;
}

bool file::is_valid() const {
  return m_impl != nullptr;
}

inline const string& file::name() const & {
  return m_name;
}

inline memory::allocator* file::allocator() const {
  return m_allocator;
}

optional<vector<rx_byte>> read_binary_file(memory::allocator* _allocator, const char* _file_name) {
  if (file open_file{_file_name, "rb"}) {
    return read_binary_stream(_allocator, &open_file);
  }

  logger(log::level::k_error, "failed to open file '%s' [%s]", _file_name,
    strerror(errno));

  return nullopt;
}

optional<vector<rx_byte>> read_text_file(memory::allocator* _allocator, const char* _file_name) {
  if (file open_file{_file_name, "rb"}) {
    return read_text_stream(_allocator, &open_file);
  }

  logger(log::level::k_error, "failed to open file '%s' [%s]", _file_name,
    strerror(errno));

  return nullopt;
}

} // namespace rx::filesystem
