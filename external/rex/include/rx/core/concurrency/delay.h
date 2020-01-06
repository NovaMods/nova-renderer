#ifndef RX_CORE_CONCURRENCY_DELAY_H
#define RX_CORE_CONCURRENCY_DELAY_H
#include "rx/core/types.h"

namespace rx::concurrency {

void delay_this_thread(rx_u64 _milliseconds);

} // namespace rx::concurrency

#endif // RX_CORE_CONCURRENCY_DELAY_H
