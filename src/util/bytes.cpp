#include "nova_renderer/util/bytes.hpp"

#include <ostream>

namespace nova::mem {
    std::ostream& operator<<(std::ostream& os, const Bytes b) {
        os << b.b_count() << "b";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const KBytes b) {
        os << b.k_count() << "kb";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const MBytes b) {
        os << b.m_count() << "mb";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const GBytes b) {
        os << b.g_count() << "gb";
        return os;
    }
} // namespace nova::mem
