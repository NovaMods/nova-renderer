#include <stdlib.h> // abort

#include "rx/core/abort.h"
#include "rx/core/global.h"
#include "rx/core/log.h"

RX_LOG("abort", logger);

namespace rx {

[[noreturn]]
void abort(const char* _message) {
  logger(log::level::k_error, "%s", _message);
#if defined(RX_PLATFORM_WINDOWS)
  ::exit(3);
#else
  // use system abort
  ::abort();
#endif
}

} // namespace rx
