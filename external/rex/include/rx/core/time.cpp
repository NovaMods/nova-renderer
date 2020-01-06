#include <SDL.h>

#include "rx/core/time.h"

namespace rx {

rx_u64 query_performance_counter_ticks() {
  return SDL_GetPerformanceCounter();
}

rx_u64 query_performance_counter_frequency() {
  return SDL_GetPerformanceFrequency();
}

} // namespace rx::core
