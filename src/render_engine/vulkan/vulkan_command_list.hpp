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
        VkCommandBuffer cmds;

        VulkanCommandList(VkCommandBuffer cmds, const VulkanRenderEngine& render_engine);

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

        void bind_pipeline(const Pipeline* pipeline) override final;

        void bind_descriptor_sets(const std::vector<DescriptorSet*>& descriptor_sets,
                                  const PipelineInterface* pipeline_interface) override final;

        void bind_vertex_buffers(const std::vector<Buffer*>& buffers) override final;

        void bind_index_buffer(const Buffer* buffer) override final;

        void draw_indexed_mesh() override final;

    private:
        const VulkanRenderEngine& render_engine;
    };
} // namespace nova::renderer::rhi
