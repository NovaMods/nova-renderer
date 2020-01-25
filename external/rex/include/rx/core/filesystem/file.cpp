#include <stdio.h> // FILE, f{open,close,read,write,seek,flush}
#include <errno.h> // errno
#include <string.h> // strcmp, strerror

#include "rx/core/log.h"
#include "rx/core/assert.h"
#include "rx/core/config.h"
#include "rx/core/hints/unlikely.h"
#include "rx/core/filesystem/file.h"

RX_LOG("filesystem/file", logger);

namespace rx::filesystem {

file::file(void* _impl, const char* _file_name, const char* _mode)
  : m_impl{_impl}
  , m_file_name{_file_name}
  , m_mode{_mode}
{
}

file::file(const char* _file_name, const char* _mode)
  : m_impl{static_cast<void*>(fopen(_file_name, _mode))}
  , m_file_name{_file_name}
  , m_mode{_mode}
{
}

file::file(file&& other_)
  : m_impl{other_.m_impl}
  , m_file_name{other_.m_file_name}
  , m_mode{other_.m_mode}
{
  other_.m_impl = nullptr;
  other_.m_file_name = nullptr;
  other_.m_mode = nullptr;
}

file::~file() {
  close();
}

rx_u64 file::read(rx_byte* data, rx_u64 size) {
  RX_ASSERT(m_impl, "invalid");
  RX_ASSERT(strcmp(m_mode, "rb") == 0 || strcmp(m_mode, "r") == 0,
    "cannot read with mode '%s'", m_mode);
  return fread(data, 1, size, static_cast<FILE*>(m_impl));
}

rx_u64 file::write(const rx_byte* data, rx_u64 size) {
  RX_ASSERT(m_impl, "invalid");
  RX_ASSERT(strcmp(m_mode, "wb")  == 0, "cannot write with mode '%s'", m_mode);
  return fwrite(data, 1, size, static_cast<FILE*>(m_impl));
}

bool file::seek(rx_u64 where) {
  RX_ASSERT(m_impl, "invalid");
  RX_ASSERT(strcmp(m_mode, "rb") == 0, "cannot seek with mode '%s'", m_mode);
  return fseek(static_cast<FILE*>(m_impl), static_cast<long>(where), SEEK_SET) == 0;
}

bool file::flush() {
  RX_ASSERT(m_impl, "invalid");
  RX_ASSERT(strcmp(m_mode, "w") == 0 || strcmp(m_mode, "wb") == 0,
    "cannot flush with mode '%s'", m_mode);
  return fflush(static_cast<FILE*>(m_impl)) == 0;
}

optional<rx_u64> file::size() {
  RX_ASSERT(m_impl, "invalid");
  RX_ASSERT(strcmp(m_mode, "rb") == 0, "cannot get size with mode '%s'", m_mode);

  auto fp{static_cast<FILE*>(m_impl)};
  if (RX_HINT_UNLIKELY(fseek(fp, 0, SEEK_END) != 0)) {
    return nullopt;
  }

  auto result{ftell(fp)};
  if (RX_HINT_UNLIKELY(result == -1L)) {
    fseek(fp, 0, SEEK_SET);
    return nullopt;
  }

  fseek(fp, 0, SEEK_SET);
  return result;
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

optional<vector<rx_byte>> read_binary_file(memory::allocator* _allocator, const char* _file_name) {
  file open_file{_file_name, "rb"};
  if (!open_file) {
    logger(log::level::k_error, "failed to open file '%s' [%s]", _file_name,
      strerror(errno));
    return nullopt;
  }

  const auto size{open_file.size()};
  if (size) {
    vector<rx_byte> data{_allocator, *size};
    if (!open_file.read(data.data(), data.size())) {
      logger(log::level::k_error, "failed to read file '%s' [%s]", _file_name,
        strerror(errno));
      return nullopt;
    }
    return data;
  } else {
    // NOTE: taking advantage of stdio buffering here to make this reasonable
    vector<rx_byte> data{_allocator, 1};
    for(rx_byte* cursor{data.data()}; open_file.read(cursor, 1); cursor++) {
      data.resize(data.size() + 1);
    }
    return data;
  }

  return nullopt;
}

static vector<rx_byte> convert_text_encoding(vector<rx_byte>&& data_) {
  // Ensure the data contains a null-terminator.
  if (data_.last() != 0) {
    data_.push_back(0);
  }

  const bool utf16_le{data_.size() >= 2 && data_[0] == 0xFF && data_[1] == 0xFE};
  const bool utf16_be{data_.size() >= 2 && data_[0] == 0xFE && data_[1] == 0xFF};
  // UTF-16.
  if (utf16_le || utf16_be) {
    // Remove the BOM.
    data_.erase(0, 2);

    rx_u16* contents{reinterpret_cast<rx_u16*>(data_.data())};
    const rx_size chars{data_.size() / 2};
    if (utf16_be) {
      // Swap the bytes around in the contents to convert BE to LE.
      for (rx_size i{0}; i < chars; i++) {
        contents[i] = (contents[i] >> 8) | (contents[i] << 8);
      }
    }

    // Determine how many bytes are needed to convert the encoding.
    const rx_size length{utf16_to_utf8(contents, chars, nullptr)};

    // Convert UTF-16 to UTF-8.
    vector<rx_byte> result{data_.allocator(), length};
    utf16_to_utf8(contents, chars, reinterpret_cast<char*>(result.data()));
    return result;
  } else if (data_.size() >= 3 && data_[0] == 0xEF && data_[1] == 0xBB && data_[2] == 0xBF) {
    // Remove the BOM.
    data_.erase(0, 3);
  }

  return data_;
}

optional<vector<rx_byte>> read_text_file(memory::allocator* _allocator, const char* _file_name) {
 if (auto result{read_binary_file(_allocator, _file_name)}) {
    // Convert the given byte stream into a compatible UTF-8 encoding. This will
    // introduce a null-terminator, strip Unicode BOMs and convert UTF-16
    // encoding to UTF-8.
    auto data{convert_text_encoding(utility::move(*result))};

#if defined(RX_PLATFORM_WINDOWS)
    // Only Windows has the odd choice of using CRLF for text files. Load the
    // contents in as binary and do a removing all instances of CR.
    auto next{reinterpret_cast<rx_byte*>(memchr(data.data(), '\r', data.size()))};

    // Leverage the use of optimized memchr to skip through large swaths of
    // binary data quickly, rather than the more obvious per-byte approach here.
    while (next) {
      const rx_ptrdiff index{next - data.data()};
      data.erase(index, index + 1);
      next = reinterpret_cast<rx_byte*>(memchr(next + 1, '\r', data.size() - index));
    }
#endif
    return data;
  }
  return nullopt;
}

} // namespace rx::filesystem
