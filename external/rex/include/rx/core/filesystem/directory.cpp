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
  WIN32_FIND_DATAW find_data;
  HANDLE handle;
};
#endif

directory::directory(memory::allocator* _allocator, string&& path_)
  : m_allocator{_allocator}
  , m_path{utility::move(path_)}
{
#if defined(RX_PLATFORM_POSIX)
  m_impl = reinterpret_cast<void*>(opendir(m_path.data()));
#elif defined(RX_PLATFORM_WINDOWS)
  // The only thing we can cache between reuses of a directory object is the
  // path conversion and the initial find handle on Windows. Subsequent reuses
  // will need to reopen the directory.
  find_context* context = m_allocator->create<find_context>();

  // Convert |m_path| to UTF-16 for Windows.
  const wide_string path_utf16 = m_path.to_utf16();
  static constexpr const wchar_t k_path_extra[] = L"\\*";
  vector<rx_u16> path_data{m_allocator, path_utf16.size() + sizeof k_path_extra,
    utility::uninitialized{}};

  memcpy(path_data.data(), path_utf16.data(), path_utf16.size() * 2);
  memcpy(path_data.data() + path_utf16.size(), k_path_extra, sizeof k_path_extra);

  // Execute one FindFirstFileW to check if the directory exists.
  const auto path = reinterpret_cast<const LPCWSTR>(path_data.data());
  const HANDLE handle = FindFirstFileW(path, &context->find_data);
  if (handle != INVALID_HANDLE_VALUE) {
    // The directory exists and has been opened. Cache the handle and the path
    // conversion for |each|.
    context->handle = handle;
    context->path_data = utility::move(path_data);
    m_impl = reinterpret_cast<void*>(context);
  } else {
    m_allocator->destroy<find_context>(context);
    m_impl = nullptr;
  }
#endif
}

directory::~directory() {
#if defined(RX_PLATFORM_POSIX)
  if (m_impl) {
    closedir(reinterpret_cast<DIR*>(m_impl));
  }
#elif defined(RX_PLATFORM_WINDOWS)
  if (m_impl) {
    m_allocator->destroy<find_context>(m_impl);
  }
#endif
}

void directory::each(function<void(item&&)>&& _function) {
  RX_ASSERT(m_impl, "directory not opened");

#if defined(RX_PLATFORM_POSIX)
  auto dir = reinterpret_cast<DIR*>(m_impl);
  struct dirent* next = readdir(dir);

  // Possible if the directory is removed between subsequent calls to |each|.
  if (!next) {
    // The directory is no longer valid, let operator bool reflect this.
    closedir(dir);
    m_impl = nullptr;
  }

  while (next) {
    // Skip '.' and '..'.
    while (next && next->d_name[0] == '.' && !(next->d_name[1 + (next->d_name[1] == '.')])) {
      next = readdir(dir);
    }

    if (next) {
      // Only accept regular files and directories, symbolic links are not allowed.
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
  auto* context = reinterpret_cast<find_context*>(m_impl);

  // The handle has been closed, this can only happen when reusing the directory
  // object, i.e multiple calls to |each|.
  if (context->handle == INVALID_HANDLE_VALUE) {
    // Attempt to reopen the directory, since Windows lacks rewinddir.
    const auto path = reinterpret_cast<const LPCWSTR>(context->path_data.data());
    const auto handle = FindFirstFileW(path, &context->find_data);
    if (handle != INVALID_HANDLE_VALUE) {
      context->handle = handle;
    } else {
      // Destroy the context and clear |m_impl| out so operator bool reflects this.
      m_allocator->destroy<find_context>(context);
      m_impl = nullptr;
      return;
    }
  }

  // Enumerate each file in the directory.
  for (;;) {
    // Skip '.' and '..'.
    if (context->find_data.cFileName[0] == L'.'
      && !context->find_data.cFileName[1 + !!(context->find_data.cFileName[1] == L'.')])
    {
      if (!FindNextFileW(context->handle, &context->find_data)) {
        break;
      }
      continue;
    }

    const wide_string utf16_name = reinterpret_cast<rx_u16*>(&context->find_data.cFileName);
    const item::type kind = context->find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
      ? item::type::k_directory
      : item::type::k_file;

    _function({utf16_name.to_utf8(), kind});

    if (!FindNextFileW(context->handle, &context->find_data)) {
      break;
    }
  }

  // There's no way to rewinddir on Windows, so just close the the find handle
  // and clear it out in the context so subequent calls to |each| reopen it
  // instead.
  FindClose(context->handle);
  context->handle = INVALID_HANDLE_VALUE;
#endif
}

} // namespace rx::filesystem
