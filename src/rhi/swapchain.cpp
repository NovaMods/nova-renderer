#include "nova_renderer/rhi/swapchain.hpp"

namespace nova::renderer::rhi {
    Swapchain::Swapchain(const uint32_t num_images, const glm::uvec2& size) : num_images(num_images), size(size) {}

    Framebuffer* Swapchain::get_framebuffer(const uint32_t frame_idx) const { return framebuffers[frame_idx]; }

    Image* Swapchain::get_image(const uint32_t frame_idx) const { return swapchain_images[frame_idx]; }

    Fence* Swapchain::get_fence(const uint32_t frame_idx) const { return fences[frame_idx]; }

    glm::uvec2 Swapchain::get_size() const { return size; }
} // namespace nova::renderer::rhi