#pragma once

#include <vulkan/vulkan.h>

#include "nova_renderer/rhi/command_list.hpp"
#include "nova_renderer/rhi/rhi_enums.hpp"
#include "nova_renderer/rhi/rhi_types.hpp"

namespace nova::renderer::rhi {
    class VulkanRenderEngine;

    /*!
     * \brief Vulkan implementation of `command_list`
     */
    class VulkanCommandList final : public CommandList {
    public:
        VkCommandBuffer cmds;

        VulkanCommandList(VkCommandBuffer cmds, const VulkanRenderEngine* render_engine);

        void resource_barriers(PipelineStageFlags stages_before_barrier,
                               PipelineStageFlags stages_after_barrier,
                               const std::pmr::vector<ResourceBarrier>& barriers) override;

        void copy_buffer(Buffer* destination_buffer,
                         mem::Bytes destination_offset,
                         Buffer* source_buffer,
                         mem::Bytes source_offset,
                         mem::Bytes num_bytes) override;

        void execute_command_lists(const std::pmr::vector<CommandList*>& lists) override;

        void begin_renderpass(Renderpass* renderpass, Framebuffer* framebuffer) override;

        void end_renderpass() override;

        void bind_pipeline(const Pipeline* pipeline) override;

        void bind_descriptor_sets(const std::pmr::vector<DescriptorSet*>& descriptor_sets, const PipelineInterface* pipeline_interface) override;

        void bind_vertex_buffers(const std::pmr::vector<Buffer*>& buffers) override;

        void bind_index_buffer(const Buffer* buffer) override;

        void draw_indexed_mesh(uint32_t num_indices, uint32_t num_instances) override;

    private:
        const VulkanRenderEngine& render_engine;
    };
} // namespace nova::renderer::rhi
