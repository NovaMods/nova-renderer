/*!
 * \author ddubois
 * \date 30-Mar-19.
 */

#pragma once

#include <nova_renderer/command_list.hpp>

#include <vulkan/vulkan.h>

namespace nova::renderer {
    class vulkan_render_engine;

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
        explicit vulkan_command_list(VkCommandBuffer cmds, const vulkan_render_engine& render_engine);

        void resource_barriers(pipeline_stage_flags stages_before_barrier,
                              pipeline_stage_flags stages_after_barrier,
                              const std::vector<resource_barrier_t>& barriers) override;

        void copy_buffer(resource_t* destination_buffer,
                         uint64_t destination_offset,
                         resource_t* source_buffer,
                         uint64_t source_offset,
                         uint64_t num_bytes) override;

        void execute_command_lists(const std::vector<command_list*>& lists) override;

        void begin_renderpass() override;
        void end_renderpass() override;
        void bind_pipeline() override;
        void bind_material() override;

        void bind_vertex_buffers() override;
        void bind_index_buffer() override;
        void draw_indexed_mesh() override;

    private:
        VkCommandBuffer cmds;
        const vulkan_render_engine& render_engine;
    };
#pragma endregion
} // namespace nova::renderer
