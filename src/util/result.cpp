#include "nova_renderer/util/result.hpp"

#include <fmt/format.h>

namespace nova::renderer {

    NovaError::NovaError(std::string message) : message(std::move(message)) {}

    NovaError::NovaError(std::string message, NovaError cause) : message(std::move(message)) {
        this->cause = std::make_unique<NovaError>();
        *this->cause = std::forward<NovaError>(cause);
    }

    std::string NovaError::to_string() const {
        if(cause) {
            return fmt::format(fmt("{:s}\nCaused by: {:s}"), message, cause->to_string());
        } else {
            return message;
        }
    }
} // namespace nova::renderer
