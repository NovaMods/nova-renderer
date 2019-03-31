/*!
 * \author ddubois 
 * \date 30-Mar-19.
 */

#pragma once

#include <nova_renderer/command_list.hpp>

#include <vulkan/vulkan.h>

namespace nova::renderer {
#pragma region Opaque pointers
    struct resource_t {
        enum class type {
            IMAGE,
            BUFFER,
        };

        type resource_type;

        union {
            VkBuffer buffer;
            VkImage image;
        };
    };
#pragma endregion

#pragma region Vulkan command list implementation
    /*!
     * \brief Vulkan implementation of `command_list`
     */
    class vulkan_command_list : public command_list {
    public:
        explicit vulkan_command_list(VkCommandBuffer cmds);

        void resource_barrier(const std::vector<resource_barrier_t>& barriers) override;

        void copy_buffer() override;

        void execute_command_lists() override;

        void begin_renderpass() override;
        void end_renderpass() override;
        void bind_pipeline() override;
        void bind_material() override;

        void bind_vertex_buffers() override;
        void bind_index_buffer() override;
        void draw_indexed_mesh() override;

    private:
        VkCommandBuffer cmds;
    };
#pragma endregion
}
