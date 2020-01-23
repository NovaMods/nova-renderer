#include "rx/core/concurrency/yield.h"
#include "rx/core/config.h" // RX_PLATFORM_WINDOWS, RX_PLATFORM_LINUX

#if defined(RX_PLATFORM_WINDOWS)
#define _WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h> // SwitchToThread
#elif defined(RX_PLATFORM_POSIX)
#include <sched.h> // sched_yield
#else
#error "missing implementation of yield"
#endif

namespace rx::concurrency {

void yield() {
#if defined(RX_PLATFORM_WINDOWS)
  SwitchToThread();
#elif defined(RX_PLATFORM_POSIX)
  sched_yield();
#endif
}

} // namespace rx::concurrency