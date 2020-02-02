#include "rx/core/concurrency/thread.h"
#include "rx/core/concurrency/atomic.h"
#include "rx/core/memory/system_allocator.h"
#include "rx/core/string.h"
#include "rx/core/profiler.h"

#if defined(RX_PLATFORM_WINDOWS)
#include <process.h> // _beginthreadex
#endif // defined(RX_PLATFORM_WINDOWS)

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
  const int thread_id{g_thread_id++};
  auto self{reinterpret_cast<state*>(_data)};
  profiler::instance().set_thread_name(self->m_name);
  self->m_function(utility::move(thread_id));
  return nullptr;
}

thread::state::state()
  : m_joined{false}
{
}

thread::state::state(const char* _name, function<void(int)>&& function_)
  : m_function{utility::move(function_)}
  , m_joined{false}
  , m_name{_name}
{
#if defined(RX_PLATFORM_POSIX)
  if (pthread_create(&m_thread, nullptr, wrap, reinterpret_cast<void*>(this)) != 0) {
    RX_ASSERT(false, "thread creation failed");
  }

  pthread_setname_np(m_thread, _name);

#elif defined(RX_PLATFORM_WINDOWS)
  // _beginthreadex on windows expects unsigned int return and __stdcall,
  auto wrap_win32{[](void* _data) -> unsigned {
    wrap(_data);
    return 0;
  }};
  m_thread = reinterpret_cast<HANDLE>(
    _beginthreadex(nullptr, 0, wrap_win32, reinterpret_cast<void*>(this), 0, nullptr));
  RX_ASSERT(m_thread, "thread creation failed");
  const wide_string converted_name{string(_name).to_utf16()};
  SetThreadDescription(m_thread, reinterpret_cast<PCWSTR>(converted_name.data()));
#endif
}

void thread::state::join() {
  if (m_joined) {
    return;
  }

#if defined(RX_PLATFORM_POSIX)
  if (pthread_join(m_thread, nullptr) != 0) {
    RX_ASSERT(false, "join failed");
  }
#elif defined(RX_PLATFORM_WINDOWS)
  if (WaitForSingleObject(m_thread, INFINITE) != WAIT_OBJECT_0) {
    RX_ASSERT(false, "join failed");
  }
#endif

  m_joined = true;
}

} // namespace rx::concurrency
