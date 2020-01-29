#include "nova_renderer/util/result.hpp"

namespace ntl {
    NovaError::NovaError(rx::string message) : message(rx::utility::move(message)) {}

    NovaError::NovaError(rx::string message, NovaError* cause) : message(rx::utility::move(message)), cause(cause) {}

    rx::string NovaError::to_string() const {
        if(cause) {
            return rx::string::format("%s\nCaused by: %s", message, cause->to_string());
        } else {
            return message;
        }
    }
} // namespace ntl
