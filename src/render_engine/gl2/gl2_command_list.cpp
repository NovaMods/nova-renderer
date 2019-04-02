/*!
 * \author ddubois
 * \date 31-Mar-19.
 */

#include "gl2_command_list.hpp"

namespace nova::renderer {
    gl2_command_list::gl2_command_list() {
        // TODO: maintain an average of the number of commands per command list, and allocate enough commands for like 90% of all command
        // lists
        commands.reserve(128);
    }

    void gl2_command_list::resource_barriers([[maybe_unused]] pipeline_stage_flags stages_before_barrier,
                                            [[maybe_unused]] pipeline_stage_flags stages_after_barrier,
                                            [[maybe_unused]] const std::vector<resource_barrier_t>& barriers) {
        // Don't need to do anything whoop
    }

    void gl2_command_list::copy_buffer(resource_t* destination_buffer,
                                       const uint64_t destination_offset,
                                       resource_t* source_buffer,
                                       const uint64_t source_offset,
                                       const uint64_t num_bytes) {
        commands.emplace_back();

        gl_command& copy_command = commands.front();
        copy_command.type = gl2_command_type::BUFFER_COPY;
        copy_command.buffer_copy.destination_buffer = destination_buffer->id;
        copy_command.buffer_copy.destination_offset = destination_offset;
        copy_command.buffer_copy.source_buffer = source_buffer->id;
        copy_command.buffer_copy.source_offset = source_offset;
        copy_command.buffer_copy.num_bytes = num_bytes;
    }

    void gl2_command_list::execute_command_lists(const std::vector<command_list*>& lists) {
        commands.emplace_back();

        gl_command& execute_lists_command = commands.front();
        execute_lists_command.execute_command_lists.lists_to_execute = lists;
    }

    std::vector<gl_command> gl2_command_list::get_commands() const { return commands; }
} // namespace nova::renderer
