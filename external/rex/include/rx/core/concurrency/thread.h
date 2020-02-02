#ifndef RX_CORE_CONCURRENCY_THREAD_H
#define RX_CORE_CONCURRENCY_THREAD_H
#include "rx/core/config.h" // RX_PLATFORM_*
#include "rx/core/function.h"

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

// NOTE: thread names must be static strings
struct thread
  : concepts::no_copy
{
  constexpr thread();

  thread(memory::allocator* _allocator, const char* _name, function<void(int)>&& function_);
  thread(const char* _name, function<void(int)>&& function_);
  thread(thread&& thread_);
  ~thread();

  void join();

private:
  struct state {
    static void* wrap(void* data);

    state();
    state(const char* _name, function<void(int)>&& function_);

    void join();

#if defined(RX_PLATFORM_POSIX)
    pthread_t m_thread;
#elif defined(RX_PLATFORM_WINDOWS)
    HANDLE m_thread;
#endif

    function<void(int)> m_function;
    bool m_joined;
    const char* m_name;
  };

  memory::allocator* m_allocator;
  state* m_state;
};

inline constexpr thread::thread()
  : m_allocator{nullptr}
  , m_state{nullptr}
{
}

inline thread::thread(const char* _name, function<void(int)>&& function_)
  : thread{&memory::g_system_allocator, _name, utility::move(function_)}
{
}

inline thread::thread(thread&& thread_)
  : m_allocator{thread_.m_allocator}
  , m_state{thread_.m_state}
{
  thread_.m_allocator = nullptr;
  thread_.m_state = nullptr;
}

} // namespace rx::concurrency

#endif // RX_CORE_CONCURRENCY_THREAD_H
