/*!
 * \author ddubois
 * \date 28-Apr-18.
 */

#ifndef NOVA_RENDERER_FRAMEBUFFER_MANAGER_H
#define NOVA_RENDERER_FRAMEBUFFER_MANAGER_H

#include <cstdint>
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>
#include "../../util/utils.hpp"

namespace nova::renderer {
    class vulkan_render_engine;

    NOVA_EXCEPTION(swapchain_creation_failed);
    NOVA_EXCEPTION(present_failed);

    /*!
     * \brief Deals with the swapchain, yo
     *
     * Methods to get he next swapchain image and whatnot are found here
     *
     * You can even get the framebuffer constructed from the current swapchain. Wow!
     */
    class swapchain_manager {
    public:
        swapchain_manager(uint32_t num_swapchain_images,
                          vulkan_render_engine& render_engine,
                          glm::ivec2 window_dimensions,
                          const std::vector<VkPresentModeKHR>& present_modes);

        void present_current_image(VkSemaphore wait_semaphores) const;

        /*!
         * \brief Acquires the next image in the swapchain, signalling the provided semaphore when the image is ready 
         * to be rendered to
         * 
         * \param image_acquire_semaphore The semaphore to signal when the image is ready to be rendered to
         */
        void acquire_next_swapchain_image(VkSemaphore image_acquire_semaphore);

        void set_current_layout(VkImageLayout new_layout);

        VkFramebuffer get_current_framebuffer();
        VkFence get_current_frame_fence();

        VkImage get_current_image();
        VkImageLayout get_current_layout();
        [[nodiscard]] VkExtent2D get_swapchain_extent() const;
        [[nodiscard]] VkFormat get_swapchain_format() const;

        // I've had a lot of bugs with RAII so here's an explicit cleanup method
        void deinit();

        [[nodiscard]] uint32_t get_current_index() const;
        [[nodiscard]] uint32_t get_num_images() const;

    private:
        vulkan_render_engine& render_engine;

        VkSwapchainKHR swapchain{};
        VkExtent2D swapchain_extent;
        VkPresentModeKHR present_mode;
        VkFormat swapchain_format;

        std::vector<VkFramebuffer> framebuffers;
        std::vector<VkImageView> swapchain_image_views;
        std::vector<VkImage> swapchain_images;
        std::vector<VkImageLayout> swapchain_image_layouts;
        std::vector<VkFence> fences;

        uint32_t num_swapchain_images;
        uint32_t cur_swapchain_index = 0;

        static VkSurfaceFormatKHR choose_surface_format(const std::vector<VkSurfaceFormatKHR>& formats);

        static VkPresentModeKHR choose_present_mode(const std::vector<VkPresentModeKHR>& modes);

        static VkExtent2D choose_surface_extent(const VkSurfaceCapabilitiesKHR& caps, const glm::ivec2& window_dimensions);

        void transition_swapchain_images_into_correct_layout(const std::vector<VkImage>& images) const;
    };
} // namespace nova::renderer

#endif // NOVA_RENDERER_FRAMEBUFFER_MANAGER_H