#include "nova_renderer/rhi/swapchain.hpp"

namespace nova::renderer::rhi {
    Swapchain::Swapchain(const uint32_t num_images_in, const glm::uvec2& size) : num_images(num_images_in), size(size) {}

    RhiFramebuffer* Swapchain::get_framebuffer(const uint32_t frame_idx) const { return framebuffers[frame_idx].get(); }

    RhiImage* Swapchain::get_image(const uint32_t frame_idx) const { return swapchain_images[frame_idx].get(); }

    RhiFence* Swapchain::get_fence(const uint32_t frame_idx) const { return fences[frame_idx].get(); }

    glm::uvec2 Swapchain::get_size() const { return size; }
} // namespace nova::renderer::rhi