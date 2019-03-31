/*!
 * \author ddubois 
 * \date 30-Mar-19.
 */

#pragma once

#include <nova_renderer/command_list.hpp>

#include <vulkan/vulkan.h>

namespace nova::renderer {
    /*!
     * \brief Vulkan implementation of `command_list`
     */
    class vulkan_command_list : public graphics_command_list {
    public:
        explicit vulkan_command_list(VkCommandBuffer cmds);

        void begin() override;

        void resource_barrier() override;

        void copy_buffer() override;

        void execute_command_lists() override;

        void begin_renderpass() override;
        void end_renderpass() override;
        void bind_pipeline() override;
        void bind_material() override;

        void bind_vertex_buffers() override;
        void bind_index_buffers() override;
        void draw_indexed() override;

    private:
        VkCommandBuffer cmds;
    };
}
