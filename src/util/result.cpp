#include "result.hpp"

namespace nova::renderer {

    nova_error::nova_error(std::string&& message) : message(message) {}

    nova_error::nova_error(std::string&& message, nova_error&& cause) : message(message) {
        this->cause = std::make_unique<nova_error>();
        *this->cause = std::forward<nova_error>(cause);
    }
} // namespace nova::renderer
