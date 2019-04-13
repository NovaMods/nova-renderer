/*!
 * \author ddubois
 * \date 30-Mar-19.
 */

#pragma once

#include <nova_renderer/command_list.hpp>

#include <vulkan/vulkan.h>

namespace nova::renderer::rhi {
    class VulkanRenderEngine;

    /*!
     * \brief Vulkan implementation of `command_list`
     */
    class VulkanCommandList final : public CommandList {
    public:
        explicit VulkanCommandList(VkCommandBuffer cmds, const VulkanRenderEngine& render_engine);

        void resource_barriers(PipelineStageFlags stages_before_barrier,
                              PipelineStageFlags stages_after_barrier,
                              const std::vector<ResourceBarrier>& barriers) override final;

        void copy_buffer(Buffer* destination_buffer,
                         uint64_t destination_offset,
                         Buffer* source_buffer,
                         uint64_t source_offset,
                         uint64_t num_bytes) override final;

        void execute_command_lists(const std::vector<CommandList*>& lists) override final;

        void begin_renderpass(Renderpass* renderpass, Framebuffer* framebuffer) override final;
        void end_renderpass() override final;
        void bind_pipeline() override final;
        void bind_material() override final;

        void bind_vertex_buffers() override final;
        void bind_index_buffer() override final;
        void draw_indexed_mesh() override final;

    private:
        VkCommandBuffer cmds;
        const VulkanRenderEngine& render_engine;
    };
} // namespace nova::renderer
