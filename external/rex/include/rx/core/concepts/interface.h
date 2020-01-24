#ifndef RX_CORE_CONCEPTS_INTERFACE_H
#define RX_CORE_CONCEPTS_INTERFACE_H
#include "rx/core/concepts/no_copy.h" // no_copy
#include "rx/core/concepts/no_move.h" // no_move

namespace rx::concepts {

// interfaces are non-copyable, non-movable and virtual
#undef interface

struct interface
  : no_copy
  , no_move
{
  virtual ~interface() = 0;
};

inline interface::~interface() {
  // { empty }
}

} // namespace rx::concepts

#endif // RX_CORE_CONCEPTS_INTERFACE_H
