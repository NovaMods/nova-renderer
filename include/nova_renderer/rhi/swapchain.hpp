#pragma once
#include <glm/glm.hpp>

#include <rx/core/vector.h>
#include <rx/core/memory/allocator.h>

namespace nova::renderer::rhi {
    struct RhiFence;
    struct RhiFramebuffer;
    struct RhiImage;
    struct RhiSemaphore;

    class Swapchain {
    public:
        Swapchain(uint32_t num_images, const glm::uvec2& size);

        virtual ~Swapchain() = default;

        /*!
         * \brief Acquires the next image in the swapchain
         *
         * \return The index of the swapchain image we just acquired
         */
        virtual uint8_t acquire_next_swapchain_image(rx::memory::allocator* allocator = nullptr) = 0;

        /*!
         * \brief Presents the specified swapchain image
         */
        virtual void present(uint32_t image_idx) = 0;

        [[nodiscard]] RhiFramebuffer* get_framebuffer(uint32_t frame_idx) const;

        [[nodiscard]] RhiImage* get_image(uint32_t frame_idx) const;

        [[nodiscard]] RhiFence* get_fence(uint32_t frame_idx) const;

        [[nodiscard]] glm::uvec2 get_size() const;

    protected:
        const uint32_t num_images;
        const glm::uvec2 size;

        // Arrays of the per-frame swapchain resources. Each swapchain implementation is responsible for filling these arrays with
        // API-specific objects

        rx::vector<RhiFramebuffer*> framebuffers;
        rx::vector<RhiImage*> swapchain_images;
        rx::vector<RhiFence*> fences;
    };
} // namespace nova::renderer::rhi
