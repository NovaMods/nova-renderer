/*!
 * \author ddubois
 * \date 31-Mar-19.
 */

#include "gl2_render_engine.hpp"
#include "gl2_command_list.hpp"

namespace nova::renderer {
    command_list* gl2_render_engine::allocate_command_list(uint32_t thread_idx,
                                                           queue_type needed_queue_type,
                                                           command_list::level command_list_type) {
        return new gl2_command_list();
    }
} // namespace nova::renderer
