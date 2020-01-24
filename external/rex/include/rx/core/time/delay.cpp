#include "rx/core/config.h"
#include "rx/core/time/delay.h"

#if defined(RX_PLATFORM_POSIX)
#include <time.h> // timespec, nanosleep
#include <errno.h> // errno, EINTR
#elif defined(RX_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define VC_EXTRALEAN
#include <windows.h> // Sleep
#undef interface
#else
#error "missing delay implementation"
#endif

namespace rx::time {

void delay(rx_u64 _milliseconds) {
#if defined(RX_PLATFORM_POSIX)
  struct timespec elapsed;
  elapsed.tv_sec = _milliseconds / 1000;
  elapsed.tv_nsec = (_milliseconds % 1000) * 1000000;

  int was_error;
  do {
    struct timespec tv;
    tv.tv_sec = elapsed.tv_sec;
    tv.tv_nsec = elapsed.tv_nsec;
    was_error = nanosleep(&tv, &elapsed);
  } while (was_error && (errno == EINTR));
#elif defined(RX_PLATFORM_WINDOWS)
  Sleep(_milliseconds);
#endif
}

} // namespace rx::time
