#pragma once
#include <vector>

namespace nova::renderer::rhi {
    struct Fence;
    struct Framebuffer;
    struct Image;
    struct Semaphore;

    class Swapchain {
    public:
        virtual ~Swapchain() = default;

        /*!
         * \brief Acquires the next image in the swapchain, signalling the provided semaphore when the image is ready
         * to be rendered to
         *
         * \param signal_semaphore The semaphore to signal when the image is ready to be rendered to
         * 
         * \return The index of the swapchain image we just acquired
         */
        virtual uint32_t acquire_next_swapchain_image(Semaphore* signal_semaphore) = 0;
        
        /*!
         * \brief Presents the specified swapchain image, telling the GPU to wait for the provided semaphore before presenting
         */
        virtual void present(uint32_t image_idx, const std::vector<Semaphore*> wait_semaphores) = 0;

        [[nodiscard]] Framebuffer* get_framebuffer(uint32_t frame_idx) const;

        [[nodiscard]] Image* get_image(uint32_t index) const;

        [[nodiscard]] Fence* get_fence(uint32_t frame_idx) const;

    protected:
        // Arrays of the per-frame swapchain resources. Each swapchain implementation is responsible for filling these arrays with
        // API-specific objects

        std::vector<Framebuffer*> framebuffers;
        std::vector<Image*> swapchain_images;
        std::vector<Fence*> fences;
    };
} // namespace nova::renderer::rhi
