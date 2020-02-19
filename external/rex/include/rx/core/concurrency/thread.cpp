#include "rx/core/concurrency/thread.h"

#include "rx/core/concurrency/atomic.h"
#include "rx/core/memory/system_allocator.h"
#include "rx/core/string.h"
#include "rx/core/profiler.h"

#if defined(RX_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <windows.h> // HANDLE
#include <process.h> // _beginthreadex,
#endif // defined(RX_PLATFORM_WINDOWS)

#if defined(RX_PLATFORM_POSIX)
#include <pthread.h> // pthread_t
#elif defined(RX_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define VC_EXTRALEAN
#include <windows.h> // HANDLE
#undef interface
#else
#error "missing thread implementation"
#endif

namespace rx::concurrency {

static atomic<int> g_thread_id;

thread::thread(memory::allocator* _allocator, const char* _name, function<void(int)>&& function_)
  : m_allocator{_allocator}
{
  RX_ASSERT(m_allocator, "null allocator");

  m_state = m_allocator->create<state>(_name, utility::move(function_));
}

thread::~thread() {
  if (m_state) {
    join();
    m_allocator->destroy<state>(m_state);
  }
}

void thread::join() {
  RX_ASSERT(m_state, "join on empty thread");
  m_state->join();
}

// state
void* thread::state::wrap(void* _data) {
  const int thread_id = g_thread_id++;
  auto self = reinterpret_cast<state*>(_data);
  profiler::instance().set_thread_name(self->m_name);
  self->m_function(utility::move(thread_id));
  return nullptr;
}

thread::state::state(const char* _name, function<void(int)>&& function_)
  : m_function{utility::move(function_)}
  , m_joined{false}
  , m_name{_name}
{
#if defined(RX_PLATFORM_POSIX)
  // Spawn the thread.
  auto handle = reinterpret_cast<pthread_t*>(m_thread);
  if (pthread_create(handle, nullptr, wrap, reinterpret_cast<void*>(this)) != 0) {
    RX_ASSERT(false, "thread creation failed");
  }

  // Set the thread's name to |_name|.
  pthread_setname_np(*handle, _name);

#elif defined(RX_PLATFORM_WINDOWS)
  // |_beginthreadex| is a bit non-standard in that it expects the __stdcall
  // calling convention and to return unsigned int status. Our |wrap| function
  // is more traditional in that it returns void* and uses the same calling
  // convention as default functions. Here we produce a lambda function which
  // will wrap |wrap|.
  auto wrap_win32 = [](void* _data) -> unsigned {
    wrap(_data);
    return 0;
  };

  // Spawn the thread.
  auto thread = _beginthreadex(nullptr, 0,
    wrap_win32, reinterpret_cast<void*>(this), 0, nullptr);

  RX_ASSERT(thread, "thread construction failed");

  auto thread_handle = reinterpret_cast<HANDLE>(thread);

  // Convert thread name to UTF-16 and set the thread's name with the new
  // |SetThreadDescription| API.
  const wide_string converted_name = string(_name).to_utf16();
  SetThreadDescription(thread_handle, reinterpret_cast<PCWSTR>(converted_name.data()));

  // Store the result of |_beginthreadex| into the HANDLE storage given by
  // |m_thread|.
  *reinterpret_cast<HANDLE*>(m_thread) = thread_handle;
#endif
}

void thread::state::join() {
  if (m_joined) {
    return;
  }

#if defined(RX_PLATFORM_POSIX)
  auto handle = *reinterpret_cast<pthread_t*>(m_thread);
  if (pthread_join(handle, nullptr) != 0) {
    RX_ASSERT(false, "join failed");
  }
#elif defined(RX_PLATFORM_WINDOWS)
  auto handle = *reinterpret_cast<HANDLE*>(m_thread);

  // Wait for the thread to terminate.
  if (WaitForSingleObject(handle, INFINITE) != WAIT_OBJECT_0) {
    RX_ASSERT(false, "join failed");
  }

  // Destroy the thread object itself.
  CloseHandle(handle);
#endif

  m_joined = true;
}

} // namespace rx::concurrency
