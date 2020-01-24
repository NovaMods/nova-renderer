#include "rx/core/profiler.h"

namespace rx {

RX_GLOBAL<profiler> profiler::s_profiler{"system", "profiler"};

}
