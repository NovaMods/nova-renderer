#ifndef RX_CORE_ABORT_H
#define RX_CORE_ABORT_H

namespace rx {

[[noreturn]]
void abort(const char* _message);

} // namespace rx

#endif // RX_CORE_ABORT_H