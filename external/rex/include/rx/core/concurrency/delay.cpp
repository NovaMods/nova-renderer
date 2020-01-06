#include <SDL_timer.h> // SDL_Delay

#include "rx/core/concurrency/delay.h"

namespace rx::concurrency {

void delay_this_thread(rx_u64 _milliseconds) {
  SDL_Delay(static_cast<Uint32>(_milliseconds));
}

} // namespace rx::concurrency
