#ifndef RX_CORE_CONCEPTS_NO_COPY_H
#define RX_CORE_CONCEPTS_NO_COPY_H

namespace rx::concepts {

struct no_copy {
  no_copy() = default;
  ~no_copy() = default;
  no_copy(const no_copy&) = delete;
  void operator=(const no_copy&) = delete;
};

} // namespace rx::concepts

#endif // RX_CORE_CONCEPTS_NO_COPY_H
