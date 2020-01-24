#ifndef RX_CORE_UTILITY_NAT_H
#define RX_CORE_UTILITY_NAT_H

namespace rx::utility {

// special tag which represents a nat: "not-a-type", commonly used as
// a stand in value in a union that contains non-trivial fields to
// enable constexpr initialization
struct nat { };

} // namespace rx::utility

#endif // RX_CORE_UTILITY_NAT_H
