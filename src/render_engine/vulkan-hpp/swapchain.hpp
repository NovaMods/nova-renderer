#pragma once 

#include <cstdint>
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.hpp>
#include "nova_renderer/util/utils.hpp"

#ifdef ENABLE_VULKAN
namespace nova::renderer::rhi {
    class VulkanRenderEngine;

    NOVA_EXCEPTION(swapchain_creation_failed);
    NOVA_EXCEPTION(present_failed);

    /*!
     * \brief Deals with the swapchain, yo
     *
     * Methods to get he next swapchain image and whatnot are found here
     *
     * You can even get the framebuffer constructed from the current swapchain. Wow!
     */
    class VulkanSwapchainManager {
    public:
        VulkanSwapchainManager(uint32_t num_swapchain_images,
                             VulkanRenderEngine& render_engine,
                             glm::ivec2 window_dimensions,
                             const std::vector<vk::PresentModeKHR>& present_modes);

        void present_current_image(vk::Semaphore wait_semaphores) const;

        /*!
         * \brief Acquires the next image in the swapchain, signalling the provided semaphore when the image is ready
         * to be rendered to
         *
         * \param image_acquire_semaphore The semaphore to signal when the image is ready to be rendered to
         */
        void acquire_next_swapchain_image(vk::Semaphore image_acquire_semaphore);

        void set_current_layout(vk::ImageLayout new_layout);

        vk::Framebuffer get_framebuffer(uint32_t frame_idx);
        vk::Fence get_frame_fence(uint32_t frame_idx);

        vk::Image get_image(uint32_t frame_idx);
        vk::ImageLayout get_layout(uint32_t frame_idx);
        [[nodiscard]] vk::Extent2D get_swapchain_extent() const;
        [[nodiscard]] vk::Format get_swapchain_format() const;

        // I've had a lot of bugs with RAII so here's an explicit cleanup method
        void deinit();

        [[nodiscard]] uint32_t get_current_index() const;
        [[nodiscard]] uint32_t get_num_images() const;

    private:
        VulkanRenderEngine& render_engine;

        vk::SwapchainKHR swapchain{};
        vk::Extent2D swapchain_extent;
        vk::PresentModeKHR present_mode;
        vk::Format swapchain_format;

        std::vector<vk::Framebuffer> framebuffers;
        std::vector<vk::ImageView> swapchain_image_views;
        std::vector<vk::Image> swapchain_images;
        std::vector<vk::ImageLayout> swapchain_image_layouts;
        std::vector<vk::Fence> fences;

        uint32_t num_swapchain_images;
        uint32_t cur_swapchain_index = 0;

        static vk::SurfaceFormatKHR choose_surface_format(const std::vector<vk::SurfaceFormatKHR>& formats);
        static vk::PresentModeKHR choose_present_mode(const std::vector<vk::PresentModeKHR>& modes);
        static vk::Extent2D choose_surface_extent(const vk::SurfaceCapabilitiesKHR& caps, const glm::ivec2& window_dimensions);

        void transition_swapchain_images_into_correct_layout(const std::vector<vk::Image>& images) const;
    };
} // namespace nova::renderer;
#endif
