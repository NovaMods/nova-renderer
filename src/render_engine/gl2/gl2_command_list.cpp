/*!
 * \author ddubois 
 * \date 31-Mar-19.
 */

#include "gl2_command_list.hpp"

namespace nova::renderer {
    gl2_command_list::gl2_command_list() {
        // TODO: Profile if this is accurate
        commands.reserve(128);
    }

    void gl2_command_list::resource_barrier(const std::vector<resource_barrier_t>& barriers) {
        // Don't need to do anything whoop
    }

    std::vector<gl_command> gl2_command_list::get_commands() const {
        return commands;
    }
}
