#include "nova_renderer/util/result.hpp"

#include <fmt/format.h>

namespace nova::renderer {

    NovaError::NovaError(const std::string& message) : message(std::move(message)) {}

    NovaError::NovaError(const std::string& message, NovaError cause) : message(std::move(message)) {
        this->cause = std::make_unique<NovaError>();
        *this->cause = std::forward<NovaError>(cause);
    }

    std::string NovaError::to_string() const {
        if(cause) {
            return fmt::format(fmt("{:s}\nCaused by: {:s}"), message.c_str(), cause->to_string().c_str()).c_str();
        } else {
            return message;
        }
    }
} // namespace nova::renderer
