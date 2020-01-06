#ifndef RX_CORE_TIME_H
#define RX_CORE_TIME_H
#include "rx/core/types.h"

namespace rx {

rx_u64 query_performance_counter_ticks();
rx_u64 query_performance_counter_frequency();


} // namespace rx::core

#endif // RX_CORE_TIME_H
