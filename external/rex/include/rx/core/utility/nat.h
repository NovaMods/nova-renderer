#ifndef RX_CORE_UTILITY_NAT_H
#define RX_CORE_UTILITY_NAT_H

namespace rx::utility {

// Special tag type which represents a NAT "not-a-type". Commonly used as a
// stand in type in a union which constains non-trivial to construct members to
// enable constexpr initialization.
//
// One such example is optional<T> which has constexpr constructors because the
// uninitialized case initializes an embedded NAT.
struct nat { };

} // namespace rx::utility

#endif // RX_CORE_UTILITY_NAT_H
