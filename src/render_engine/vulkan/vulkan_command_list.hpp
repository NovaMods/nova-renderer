/*!
 * \author ddubois
 * \date 30-Mar-19.
 */

#pragma once

#include <nova_renderer/command_list.hpp>

#include <vulkan/vulkan.h>

namespace nova::renderer::rhi {
    class vk_render_engine;

    /*!
     * \brief Vulkan implementation of `command_list`
     */
    class vulkan_command_list : public command_list_t {
    public:
        explicit vulkan_command_list(VkCommandBuffer cmds, const vk_render_engine& render_engine);

        void resource_barriers(pipeline_stage_flags stages_before_barrier,
                              pipeline_stage_flags stages_after_barrier,
                              const std::vector<resource_barrier_t>& barriers) override final;

        void copy_buffer(buffer_t* destination_buffer,
                         uint64_t destination_offset,
                         buffer_t* source_buffer,
                         uint64_t source_offset,
                         uint64_t num_bytes) override final;

        void execute_command_lists(const std::vector<command_list_t*>& lists) override final;

        void begin_renderpass(renderpass_t* renderpass, framebuffer_t* framebuffer) override final;
        void end_renderpass() override final;
        void bind_pipeline() override final;
        void bind_material() override final;

        void bind_vertex_buffers() override final;
        void bind_index_buffer() override final;
        void draw_indexed_mesh() override final;

    private:
        VkCommandBuffer cmds;
        const vk_render_engine& render_engine;
    };
} // namespace nova::renderer
