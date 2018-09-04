/*!
 * \author ddubois 
 * \date 01-Sep-18.
 */

#include "command_buffer_base.hpp"

namespace nova {
    command_buffer_base::command_buffer_base(command_buffer_type type) : type(type) {

    }

    command_buffer_type command_buffer_base::get_type() const {
        return type;
    }

    graphics_command_buffer_base::graphics_command_buffer_base() : command_buffer_base(command_buffer_type::GENERIC) {

    }
}
