#include "vulkan_render_engine.hpp"

namespace nova::renderer {
    void vulkan_render_engine::create_builtin_uniform_buffers() {
        // TODO: move this to the settings
        const uint32_t static_object_estimate = 5000;
        VkBufferCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.size = static_object_estimate * sizeof(glm::mat4);
        info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        const uint32_t alignment = static_cast<uint32_t>(gpu.props.limits.minUniformBufferOffsetAlignment);

        static_model_matrix_buffer = std::make_unique<auto_buffer>("NovaStaticModelUBO", vma_allocator, info, alignment, false);
    }
} // namespace nova::renderer