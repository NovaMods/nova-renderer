#include "rx/core/config.h" // RX_PLATFORM_*
#include "rx/core/assert.h" // RX_ASSERT

#if defined(RX_PLATFORM_POSIX)
#include <dirent.h> // DIR, struct dirent, opendir, readdir, rewinddir, closedir
#elif defined(RX_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define VC_EXTRALEAN
#include <windows.h> // WIN32_FIND_DATAW, HANDLE, LPCWSTR, INVALID_HANDLE_VALUE, FILE_ATTRIBUTE_DIRECTORY, FindFirstFileW, FindNextFileW, FindClose
#undef interface
#else
#error "missing directory implementation"
#endif

#include "rx/core/filesystem/directory.h"

namespace rx::filesystem {

#if defined(RX_PLATFORM_WINDOWS)
struct find_context {
  vector<rx_u16> path_data;
};
#endif

directory::directory(memory::allocator* _allocator, string&& path_)
  : m_allocator{_allocator}
  , m_path{utility::move(path_)}
{
#if defined(RX_PLATFORM_POSIX)
  m_impl = reinterpret_cast<void*>(opendir(m_path.data()));
#elif defined(RX_PLATFORM_WINDOWS)
  // WIN32 FindData does not support "rewinding" a directory so |each| must open it each time,
  // the only thing we can cache between reuses of a directory object is the path conversion
  vector<rx_u16>* path_data{m_allocator->create<vector<rx_u16>>(m_allocator)};

  const wide_string path_utf16{m_path.to_utf16()};
  static constexpr const wchar_t k_path_extra[] = L"\\*";

  path_data->resize(path_utf16.size() + sizeof k_path_extra);
  memcpy(path_data->data(), path_utf16.data(), path_utf16.size() * 2);
  memcpy(path_data->data() + path_utf16.size(), k_path_extra, sizeof k_path_extra);

  m_impl = reinterpret_cast<void*>(path_data);
#endif
}

directory::~directory() {
#if defined(RX_PLATFORM_POSIX)
  if (m_impl) {
    closedir(reinterpret_cast<DIR*>(m_impl));
  }
#elif defined(RX_PLATFORM_WINDOWS)
  if (m_impl) {
    m_allocator->destroy<vector<rx_u16>>(m_impl);
  }
#endif
}

void directory::each(function<void(item&&)>&& _function) {
  RX_ASSERT(m_impl, "invalid directory");

#if defined(RX_PLATFORM_POSIX)
  auto dir{reinterpret_cast<DIR*>(m_impl)};
  struct dirent* next{readdir(dir)};
  while (next) {
    // skip '..'
    while (next && next->d_name[0] == '.' && !(next->d_name[1 + (next->d_name[1] == '.')])) {
      next = readdir(dir);
    }

    if (next) {
      // only accept regular files and directories, symbol links are not allowed
      switch (next->d_type) {
      case DT_DIR:
        _function({{m_allocator, next->d_name}, item::type::k_directory});
        break;
      case DT_REG:
        _function({{m_allocator, next->d_name}, item::type::k_file});
        break;
      }

      next = readdir(dir);
    } else {
      break;
    }
  }

  rewinddir(dir);
#elif defined(RX_PLATFORM_WINDOWS)
  auto* context{reinterpret_cast<find_context*>(m_impl)};

  WIN32_FIND_DATAW find_data;
  HANDLE handle{INVALID_HANDLE_VALUE};
  if (!(handle = FindFirstFileW(reinterpret_cast<LPCWSTR>(context->path_data.data()), &find_data))) {
    return;
  }

  for (;;) {
    if (find_data.cFileName[0] == L'.' && !find_data.cFileName[1 + !!(find_data.cFileName[1] == L'.')]) {
      if (!FindNextFileW(handle, &find_data)) {
        break;
      }
      continue;
    }

    const wide_string utf16_name{reinterpret_cast<rx_u16*>(&find_data.cFileName)};
    string utf8_name{utf16_name.to_utf8()};
    item::type kind{find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
      ? item::type::k_directory : item::type::k_file};
    _function({utility::move(utf8_name), kind});

    if (!FindNextFileW(handle, &find_data)) {
      break;
    }
  }

  FindClose(handle);
#endif
}
} // namespace rx::filesystem
