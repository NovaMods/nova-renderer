/*!
 * \author ddubois 
 * \date 28-Apr-18.
 */

#ifndef NOVA_RENDERER_FRAMEBUFFER_MANAGER_H
#define NOVA_RENDERER_FRAMEBUFFER_MANAGER_H

#include <cstdint>
#include <vulkan/vulkan.hpp>
#include "vulkan/render_context.h"

namespace nova {
    /*!
     * \brief Deals with the swapchain, yo
     *
     * Methods to get he next swapchain image and whatnot are found here
     *
     * You can even get the framebuffer constructed from the current swapchain. Wow!
     */
    class swapchain_manager {
    public:
        swapchain_manager(uint32_t num_swapchain_images, std::shared_ptr<render_context> context, glm::ivec2 window_dimensions);

        void present_current_image(const std::vector<vk::Semaphore>& wait_semaphores);

        void aqcuire_next_swapchain_image(vk::Semaphore image_acquire_semaphore);

        void set_current_layout(vk::ImageLayout new_layout);

        vk::Framebuffer get_current_framebuffer();

        vk::Image get_current_image();
        vk::ImageLayout get_current_layout();
        vk::Extent2D get_swapchain_extent();
        vk::Format get_swapchain_format();


        // I've had a lot of bugs with RAII so here's an explicit cleanup method
        void deinit();

    private:
        std::shared_ptr<render_context> context;

        vk::SwapchainKHR swapchain;
        vk::Extent2D swapchain_extent;
        vk::PresentModeKHR present_mode;
        vk::Format swapchain_format;
        vk::Format depth_format;

        std::vector<vk::Framebuffer> framebuffers;
        std::vector<vk::ImageView> swapchain_image_views;
        std::vector<vk::Image> swapchain_images;
        std::vector<vk::ImageLayout> swapchain_image_layouts;

        uint32_t cur_swapchain_index = 0;

        vk::SurfaceFormatKHR choose_surface_format(std::vector<vk::SurfaceFormatKHR> &formats);

        vk::PresentModeKHR choose_present_mode(std::vector<vk::PresentModeKHR> &modes);

        vk::Extent2D choose_surface_extent(vk::SurfaceCapabilitiesKHR &caps, glm::ivec2 &window_dimensions);

        vk::Format choose_supported_format(vk::Format *formats, int num_formats, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

        void move_swapchain_images_into_correct_format(const std::vector<vk::Image> &images);
    };
}

#endif //NOVA_RENDERER_FRAMEBUFFER_MANAGER_H
