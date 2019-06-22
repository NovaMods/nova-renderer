#pragma once
#include "nova_renderer/swapchain.hpp"

namespace nova::renderer::rhi {
    class Gl3Swapchain final : public Swapchain {
    public:
        explicit Gl3Swapchain(uint32_t num_swapchain_images, const glm::uvec2& size);

        ~Gl3Swapchain() override = default;

        uint32_t acquire_next_swapchain_image() override;

        void present(uint32_t image_idxs) override;

    private:
        uint32_t cur_frame = 0;
    };
}
