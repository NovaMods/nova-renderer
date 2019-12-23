#include "gl3_swapchain.hpp"

#include "gl3_structs.hpp"

namespace nova::renderer::rhi {
    Gl3Swapchain::Gl3Swapchain(const uint32_t num_swapchain_images, const glm::uvec2& size) : Swapchain(num_swapchain_images, size) {
        for(uint32_t i = 0; i < num_swapchain_images; i++) {
            framebuffers.push_back(new Gl3Framebuffer);
            swapchain_images.push_back(new Gl3Image);
            fences.push_back(new Gl3Fence);
        }
    }

    uint8_t Gl3Swapchain::acquire_next_swapchain_image() {
        const uint8_t ret_val = cur_frame;
        cur_frame++;
        if(cur_frame >= num_images) {
            cur_frame = 0;
        }

        return ret_val;
    }

    void Gl3Swapchain::present(uint32_t /* image_idx */) { glFlush(); }
} // namespace nova::renderer::rhi
