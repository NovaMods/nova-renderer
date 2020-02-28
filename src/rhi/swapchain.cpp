#include "nova_renderer/rhi/swapchain.hpp"

namespace nova::renderer::rhi {
    Swapchain::Swapchain(const uint32_t num_images, const glm::uvec2& size) : num_images(num_images), size(size) {}

    RhiFramebuffer* Swapchain::get_framebuffer(const uint32_t frame_idx) const { return framebuffers[frame_idx]; }

    RhiImage* Swapchain::get_image(const uint32_t frame_idx) const { return swapchain_images[frame_idx]; }

    RhiFence* Swapchain::get_fence(const uint32_t frame_idx) const { return fences[frame_idx]; }

    glm::uvec2 Swapchain::get_size() const { return size; }
} // namespace nova::renderer::rhi