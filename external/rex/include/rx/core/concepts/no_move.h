#ifndef RX_CORE_CONCEPTS_NO_MOVE_H
#define RX_CORE_CONCEPTS_NO_MOVE_H

namespace rx::concepts {

struct no_move {
  no_move() = default;
  ~no_move() = default;
  no_move(no_move&&) = delete;
  void operator=(no_move&&) = delete;
};

} // namespace rx::concepts

#endif // RX_CORE_CONCEPTS_NO_MOVE_H
