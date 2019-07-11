#include "nova_renderer/util/result.hpp"

#include <fmt/format.h>

namespace nova::renderer {

    NovaError::NovaError(const eastl::string& message) : message(eastl::move(message)) {}

    NovaError::NovaError(const eastl::string& message, NovaError cause) : message(eastl::move(message)) {
        this->cause = eastl::make_unique<NovaError>();
        *this->cause = eastl::forward<NovaError>(cause);
    }

    eastl::string NovaError::to_string() const {
        if(cause) {
            return fmt::format(fmt("{:s}\nCaused by: {:s}"), message.c_str(), cause->to_string().c_str()).c_str();
        } else {
            return message;
        }
    }
} // namespace nova::renderer
