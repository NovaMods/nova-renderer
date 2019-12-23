#pragma once
#include <vector>

#include <glm/glm.hpp>

#include "nova_renderer/memory/allocators.hpp"

namespace nova::renderer::rhi {
    struct Fence;
    struct Framebuffer;
    struct Image;
    struct Semaphore;

    class Swapchain {
    public:
        Swapchain(uint32_t num_images, const glm::uvec2& size);

        virtual ~Swapchain() = default;

        /*!
         * \brief Acquires the next image in the swapchain
         *
         * \return The index of the swapchain image we just acquired
         */
        virtual uint8_t acquire_next_swapchain_image(memory::AllocatorHandle<>& allocator) = 0;

        /*!
         * \brief Presents the specified swapchain image
         */
        virtual void present(uint32_t image_idx) = 0;

        [[nodiscard]] Framebuffer* get_framebuffer(uint32_t frame_idx) const;

        [[nodiscard]] Image* get_image(uint32_t frame_idx) const;

        [[nodiscard]] Fence* get_fence(uint32_t frame_idx) const;

        [[nodiscard]] glm::uvec2 get_size() const;

    protected:
        const uint32_t num_images;
        const glm::uvec2 size;

        // Arrays of the per-frame swapchain resources. Each swapchain implementation is responsible for filling these arrays with
        // API-specific objects

        std::vector<Framebuffer*> framebuffers;
        std::vector<Image*> swapchain_images;
        std::vector<Fence*> fences;
    };
} // namespace nova::renderer::rhi
