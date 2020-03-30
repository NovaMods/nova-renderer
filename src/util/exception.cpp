#include "nova_renderer/exception.hpp"

#include <rx/core/log.h>

namespace nova {
    RX_LOG("Exception", logger);

    Exception::Exception(rx::string msg) : message{rx::utility::move(msg)} { logger->error(message.data()); }

    const rx::string& Exception::get_message() const { return message; }
} // namespace nova
