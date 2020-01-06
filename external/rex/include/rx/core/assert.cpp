#include <stdlib.h> // abort
#include <stdarg.h> // va_{list,start,end,copy}
#include <stdio.h> // vsnprintf

#include "rx/core/log.h" // RX_LOG, rx::log
#include "rx/core/abort.h" // rx::abort

RX_LOG("assert", logger);

namespace rx {

[[noreturn]]
void assert_fail(const char* _expression, const char* _file,
  const char* _function, int _line, const char* _message, ...)
{
  va_list va;
  va_start(va, _message);

  // calculate length to format
  va_list ap;
  va_copy(ap, va);
  const int length{vsnprintf(nullptr, 0, _message, ap)};
  va_end(ap);

  // format into string
  string contents;
  contents.resize(length);
  vsnprintf(contents.data(), contents.size() + 1, _message, va);
  va_end(va);

  logger(log::level::k_error, "Assertion failed: %s (%s:%d %s) \"%s\"",
    _expression, _file, _line, _function, utility::move(contents));

  abort(contents.data());
}

} // namespace rx
