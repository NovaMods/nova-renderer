#include "nova_renderer/util/result.hpp"

namespace ntl {
    NovaError::NovaError(std::string message) : message(rx::utility::move(message)) {}

    NovaError::NovaError(std::string message, NovaError* cause) : message(rx::utility::move(message)), cause(cause) {}

    std::string NovaError::to_string() const {
        if(cause) {
            return std::string::format("%s\nCaused by: %s", message, cause->to_string());
        } else {
            return message;
        }
    }
} // namespace ntl
