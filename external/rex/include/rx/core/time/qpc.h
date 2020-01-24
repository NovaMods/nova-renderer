#ifndef RX_CORE_TIME_QPC_H
#define RX_CORE_TIME_QPC_H
#include "rx/core/types.h"

namespace rx::time {

rx_u64 qpc_ticks();
rx_u64 qpc_frequency();

} // namespace rx::time

#endif // RX_CORE_TIME_QPC_H
