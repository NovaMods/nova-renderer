#include "result.hpp"
#include "fmt/format.h"

namespace nova::renderer {

    nova_error::nova_error(std::string&& message) : message(message) {}

    nova_error::nova_error(std::string&& message, nova_error&& cause) : message(message) {
        this->cause = std::make_unique<nova_error>();
        *this->cause = std::forward<nova_error>(cause);
    }

    std::string nova_error::to_string() const {
        if(cause) {
            return fmt::format(fmt("{:s}\nCaused by: {:s}"), message, cause->to_string());
        }
        else {
            return message;
        }
    }
} // namespace nova::renderer
