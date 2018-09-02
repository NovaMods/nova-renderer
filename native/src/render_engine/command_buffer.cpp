/*!
 * \author ddubois 
 * \date 01-Sep-18.
 */

#include "command_buffer.hpp"

namespace nova {
    command_buffer::command_buffer(command_buffer_type type) : type(type) {

    }

    command_buffer_type command_buffer::get_type() const {
        return type;
    }
}
