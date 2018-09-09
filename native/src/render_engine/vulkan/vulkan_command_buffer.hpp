//
// Created by ddubois on 9/4/18.
//

#ifndef NOVA_RENDERER_VULKAN_COMMAND_BUFFER_H
#define NOVA_RENDERER_VULKAN_COMMAND_BUFFER_H

#include <vulkan/vulkan.h>
#include "../command_buffer_base.hpp"

namespace nova {
    class vulkan_command_buffer : public graphics_command_buffer_base {
    public:
        vulkan_command_buffer(VkDevice device, VkCommandPool command_pool, command_buffer_type type);

        void start_recording() override;

        void end_recording() override;

        void reset() override;

        void resource_barrier(stage_flags source_stage_mask, stage_flags dest_stage_mask,
                              const std::vector<resource_barrier_data>& memory_barriers,
                              const std::vector<buffer_barrier_data>& buffer_barriers,
                              const std::vector<image_barrier_data>& image_barriers) override;

        void clear_render_target(iframebuffer* framebuffer_to_clear, glm::vec4& clear_color) override;

        void set_render_target(iframebuffer* render_target) override;

        VkCommandBuffer get_vk_buffer();

    private:
        VkDevice device;
        VkCommandBuffer buffer;
    };
}

#endif //NOVA_RENDERER_VULKAN_COMMAND_BUFFER_H
