#include "nova_renderer/swapchain.hpp"

namespace nova::renderer::rhi {
    Framebuffer* Swapchain::get_framebuffer(const uint32_t frame_idx) const { return framebuffers.at(frame_idx); }

    Image* Swapchain::get_image(const uint32_t index) const { return swapchain_images.at(index); }

    Fence* Swapchain::get_fence(const uint32_t frame_idx) const { return fences.at(frame_idx); };
} // namespace nova::renderer::rhi