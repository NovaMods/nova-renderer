#include "nova_renderer/bytes.hpp"
#include <ostream>

std::ostream& bvestl::polyalloc::operator<<(std::ostream& os, Bytes b) {
    os << b.b_count() << "b";
    return os;
}

std::ostream& bvestl::polyalloc::operator<<(std::ostream& os, KBytes b) {
    os << b.k_count() << "kb";
    return os;
}

std::ostream& bvestl::polyalloc::operator<<(std::ostream& os, MBytes b) {
    os << b.m_count() << "mb";
    return os;
}

std::ostream& bvestl::polyalloc::operator<<(std::ostream& os, GBytes b) {
    os << b.g_count() << "gb";
    return os;
}
