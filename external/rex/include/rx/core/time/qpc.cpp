#include "rx/core/time/qpc.h"
#include "rx/core/config.h"

#if defined(RX_PLATFORM_POSIX)
#include <time.h> // CLOCK_MONOTONIC, struct timespec, clock_gettime
#elif defined(RX_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define VC_EXTRALEAN
#include <windows.h> // LARGE_INTEGER, QueryPerformance{Counter, Frequency}
#undef interface
#else
#error "missing qpc implementation"
#endif

namespace rx::time {

rx_u64 qpc_ticks() {
#if defined(RX_PLATFORM_POSIX)
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);

  rx_u64 ticks{0};
  ticks += now.tv_sec;
  ticks *= 1000000000;
  ticks += now.tv_nsec;

  return ticks;
#elif defined(RX_PLATFORM_WINDOWS)
  LARGE_INTEGER counter;
  QueryPerformanceCounter(&counter);
  return counter.QuadPart;
#endif
  return 0;
}

rx_u64 qpc_frequency() {
#if defined(RX_PLATFORM_POSIX)
  return 1000000000;
#elif defined(RX_PLATFORM_WINDOWS)
  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);
  return frequency.QuadPart;
#endif
}

} // namespace rx::time
